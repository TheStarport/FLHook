#include "API/FLHook/Database.hpp"
#include "PCH.hpp"

#include "Core/Commands/CommandLineParser.hpp"
#include "Defs/FLHookConfig.hpp"

#include "spdlog/details/null_mutex.h"
#include <mutex>

using MongoSinkMt = MongoSink<std::mutex>;

enum class ConsoleColor
{
    Blue = 0x0001,
    Green = 0x0002,
    Cyan = Blue | Green,
    Red = 0x0004,
    Purple = Red | Blue,
    Yellow = Red | Green,
    White = Red | Blue | Green,

    StrongBlue = 0x0008 | Blue,
    StrongGreen = 0x0008 | Green,
    StrongCyan = 0x0008 | Cyan,
    StrongRed = 0x0008 | Red,
    StrongPurple = 0x0008 | Purple,
    StrongYellow = 0x0008 | Yellow,
    StrongWhite = 0x0008 | White,
};

BOOL WINAPI ConsoleHandler(DWORD ctrlType) { return ctrlType == CTRL_CLOSE_EVENT; }

using namespace std::chrono_literals;

void FlHookLogFormatFlag::format(const spdlog::details::log_msg& m, const std::tm& tm, spdlog::memory_buf_t& dest)
{
    auto entireMessageView = std::string_view(m.payload.data(), m.payload.size());
    auto offset = entireMessageView.find(JsonLogFormatter::EoLMarker);
    if (offset == std::string::npos)
    {
        return;
    }

    auto propertiesView = std::string_view(entireMessageView.data() + offset + JsonLogFormatter::EoLMarker.size(),
                                           entireMessageView.size() - offset - JsonLogFormatter::EoLMarker.size());
    auto messageView = std::string_view(entireMessageView.data(), offset);

    auto doc = bsoncxx::from_json(propertiesView);
    auto formatted = std::string(messageView);
    for (auto prop : doc.view())
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

    doc = bsoncxx::from_json(std::format("{}}}", std::string_view(dest.begin(), dest.end())));
    auto timestamp = doc["timestamp"].get_string().value;
    auto level = doc["level"].get_string().value;

    dest.clear();
    dest.append(std::format("[{}] [{}] {}", timestamp, level, formatted));
}
std::unique_ptr<spdlog::custom_flag_formatter> FlHookLogFormatFlag::clone() const { return spdlog::details::make_unique<FlHookLogFormatFlag>(); }

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
    logger = std::make_shared<spdlog::logger>("logger");
    logger->flush_on(spdlog::level::warn);
    spdlog::flush_every(std::chrono::milliseconds(250));

    // auto& mongo = sinks.emplace_back(std::make_shared<MongoSinkMt>());
    // mongo->set_pattern(R"({"timestamp":"%Y-%m-%dT%H:%M:%S.%e%z","level":"%l"%v})");
    // logger->sinks().emplace_back(mongo);

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
    formatter->add_flag<FlHookLogFormatFlag>('#').set_pattern(R"({"level": "%l","timestamp":"%T"%#)");

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

    std::string logMsg = StringUtils::wstos(message);
    consoleSink->log({ "logger", spdlog::level::info, logMsg });
}

std::shared_ptr<spdlog::logger> Logger::GetLogger() { return logger; }
