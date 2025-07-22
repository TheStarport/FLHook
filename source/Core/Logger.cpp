#include "API/FLHook/Database.hpp"
#include "PCH.hpp"

#include "Core/Commands/CommandLineParser.hpp"
#include "Defs/FLHookConfig.hpp"

#include <mutex>

using namespace std::chrono_literals;

// Empty sink that does nothing, we use the format flag instead
// A bit hacky, but it works
class MongoSink final : public spdlog::sinks::base_sink<std::mutex>
{
        void sink_it_(const spdlog::details::log_msg& msg) override
        {
            spdlog::memory_buf_t formatted;
            formatter_->format(msg, formatted);
        }
        void flush_() override {}
};

class FLHookConsoleFormatFlag final : public spdlog::custom_flag_formatter
{
    public:
        void format(const spdlog::details::log_msg& m, const std::tm& tm, spdlog::memory_buf_t& dest) override;
        std::unique_ptr<custom_flag_formatter> clone() const override;
};

class FLHookMongoFormatFlag final : public spdlog::custom_flag_formatter
{
    public:
        void format(const spdlog::details::log_msg& m, const std::tm& tm, spdlog::memory_buf_t& dest) override;
        std::unique_ptr<custom_flag_formatter> clone() const override;
};

BOOL WINAPI ConsoleHandler(DWORD ctrlType) { return ctrlType == CTRL_CLOSE_EVENT; }

static std::string FormatMessageFromProperties(const std::string_view templateMsg, bsoncxx::document::view properties)
{
    auto formatted = std::string(templateMsg);
    for (auto prop : properties)
    {
        std::string value;
        if (prop.type() == bsoncxx::type::k_int32)
        {
            value = std::to_string(prop.get_int32().value);
        }
        else if (prop.type() == bsoncxx::type::k_int64)
        {
            value = std::to_string(prop.get_int64().value);
        }
        else if (prop.type() == bsoncxx::type::k_bool)
        {
            value = prop.get_bool().value ? "true" : "false";
        }
        else if (prop.type() == bsoncxx::type::k_double)
        {
            value = std::format("{:0.5f}", prop.get_double().value);
        }
        else if (prop.type() == bsoncxx::type::k_string)
        {
            value = std::string(prop.get_string().value);
        }
        else
        {
            continue;
        }

        formatted = StringUtils::ReplaceStr(formatted, std::string_view(std::format("{{{}}}", prop.key())), std::string_view(value));
    }

    return formatted;
}

void FLHookConsoleFormatFlag::format(const spdlog::details::log_msg& m, const std::tm& tm, spdlog::memory_buf_t& dest)
{
    auto entireMessageView = std::string_view(m.payload.data(), m.payload.size());
    const auto offset = entireMessageView.find(JsonLogFormatter::EoLMarker);
    if (offset == std::string::npos)
    {
        return;
    }

    const auto propertiesView = std::string_view(entireMessageView.data() + offset + JsonLogFormatter::EoLMarker.size(),
                                                 entireMessageView.size() - offset - JsonLogFormatter::EoLMarker.size());
    const auto messageView = std::string_view(entireMessageView.data(), offset);
    auto doc = bsoncxx::from_json(propertiesView);
    auto message = FormatMessageFromProperties(messageView, doc.view());

    doc = bsoncxx::from_json(std::format("{}}}", std::string_view(dest.begin(), dest.end())));
    auto timestamp = doc["timestamp"].get_string().value;
    auto level = doc["level"].get_string().value;

    dest.clear();
    dest.append(std::format("[{}] [{}] {}", timestamp, level, message));
}

std::unique_ptr<spdlog::custom_flag_formatter> FLHookConsoleFormatFlag::clone() const { return spdlog::details::make_unique<FLHookConsoleFormatFlag>(); }

// ReSharper disable once CppPassValueParameterByConstReference
static concurrencpp::result<void> SendToMongo(const bsoncxx::document::value document)
{
    FLHook::GetDatabase()->BeginDatabaseQuery().InsertIntoCollection(DatabaseCollection::ServerLog, { document.view() });
    co_return;
}

void FLHookMongoFormatFlag::format(const spdlog::details::log_msg& m, const std::tm& tm, spdlog::memory_buf_t& dest)
{
    auto entireMessageView = std::string_view(m.payload.data(), m.payload.size());
    const auto offset = entireMessageView.find(JsonLogFormatter::EoLMarker);
    if (offset == std::string::npos)
    {
        return;
    }

    const auto propertiesView = std::string_view(entireMessageView.data() + offset + JsonLogFormatter::EoLMarker.size(),
                                                 entireMessageView.size() - offset - JsonLogFormatter::EoLMarker.size());
    const auto messageView = std::string_view(entireMessageView.data(), offset);

    const auto doc = bsoncxx::from_json(propertiesView);
    auto message = FormatMessageFromProperties(messageView, doc.view());

    using namespace bsoncxx::builder::basic;
    FLHook::GetTaskScheduler()->ScheduleTask(
        SendToMongo,
        make_document(kvp("message", message),
                      kvp("utcTimestamp", bsoncxx::types::b_date{ std::chrono::system_clock::from_time_t(std::mktime(const_cast<std::tm*>(&tm))) }),
                      kvp("level", bsoncxx::from_json(std::format("{}}}", std::string_view(dest.begin(), dest.end())))["level"].get_string().value),
                      kvp("properties", doc.view())));

    dest.clear();
}

std::unique_ptr<spdlog::custom_flag_formatter> FLHookMongoFormatFlag::clone() const { return spdlog::details::make_unique<FLHookMongoFormatFlag>(); }

void Logger::GetConsoleInput(std::stop_token st)
{
    while (!st.stop_requested())
    {
        std::wstring cmd;
        std::getline(std::wcin >> std::ws, cmd);

        if (!cmd.empty())
        {
            commandQueue.enqueue(cmd);
        }

        std::this_thread::sleep_for(1s);
    }
}

void Logger::Init()
{
    const auto config = FLHook::GetConfig();
    logger = std::make_shared<spdlog::logger>("logger");
    logger->set_level(static_cast<spdlog::level::level_enum>(config->logging.minLogLevel));
    logger->flush_on(spdlog::level::warn);
    spdlog::flush_every(std::chrono::milliseconds(250));

    if (config->logging.logServerLogsToDatabase)
    {
        auto formatter = std::make_unique<spdlog::pattern_formatter>();
        formatter->add_flag<FLHookMongoFormatFlag>('#').set_pattern(R"({"level": "%l"%#)");

        auto mongo = std::make_shared<MongoSink>();
        mongo->set_formatter(std::move(formatter));
        logger->sinks().emplace_back(mongo);

        if (config->logging.minLogLevel <= LogLevel::Debug)
        {
            // Don't save debug logs in the db
            mongo->set_level(spdlog::level::info);
        }
    }

    spdlog::set_default_logger(logger);
    if (const CommandLineParser cmd; cmd.CmdOptionExists(L"-noconsole"))
    {
        consoleAllocated = false;
        return;
    }

    AllocConsole();
    SetConsoleTitleW(L"FLHook");

    const auto console = GetConsoleWindow();
    RECT r;
    GetWindowRect(console, &r);

    MoveWindow(console, r.left, r.top, 1366, 768, TRUE);
    freopen_s(reinterpret_cast<FILE**>(stdout), "CONOUT$", "w", stdout);
    freopen_s(reinterpret_cast<FILE**>(stdin), "CONIN$", "r", stdin);

    SetConsoleCtrlHandler(ConsoleHandler, TRUE);
    consoleInput = GetStdHandle(STD_INPUT_HANDLE);
    consoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);

    // change version number here:
    // https://patorjk.com/software/taag/#p=display&f=Doom&t=FLHook%205.0%20pallas
    std::wstring welcomeText = LR"(
______ _      _   _             _      _____  _____               _ _
|  ___| |    | | | |           | |    |  ___||  _  |             | | |
| |_  | |    | |_| | ___   ___ | | __ |___ \ | |/' |  _ __   __ _| | | __ _ ___
|  _| | |    |  _  |/ _ \ / _ \| |/ /     \ \|  /| | | '_ \ / _` | | |/ _` / __|
| |   | |____| | | | (_) | (_) |   <  /\__/ /\ |_/ / | |_) | (_| | | | (_| \__ \
\_|   \_____/\_| |_/\___/ \___/|_|\_\ \____(_)\___/  | .__/ \__,_|_|_|\__,_|___/
                                                     | |
                                                     |_|                        )";
    welcomeText += L"\n\n";
    std::wcout << welcomeText << std::flush;

    commandThread = std::jthread(std::bind_front(&Logger::GetConsoleInput));

    auto formatter = std::make_unique<spdlog::pattern_formatter>();
    formatter->add_flag<FLHookConsoleFormatFlag>('#').set_pattern(R"({"level": "%l","timestamp":"%T"%#)");

    consoleSink = logger->sinks().emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
    consoleSink->set_formatter(std::move(formatter));
}

std::optional<std::wstring> Logger::GetCommand()
{
    std::wstring cmd;
    if (!commandQueue.try_dequeue(cmd))
    {
        return {};
    }

    return StringUtils::Trim(cmd);
}

void Logger::MessageConsole(std::wstring_view message)
{
    if (!consoleSink)
    {
        return;
    }

    std::string logMsg = fmt::format("{}{}", StringUtils::wstos(message), JsonLogFormatter{});
    consoleSink->log({ "logger", spdlog::level::info, logMsg });
}

std::shared_ptr<spdlog::logger> Logger::GetLogger() { return logger; }
