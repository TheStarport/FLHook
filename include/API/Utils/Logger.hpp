#pragma once

#include "Core/FLHook.hpp"
#include "API/FLHook/Database.hpp"
#include "API/FLHook/TaskScheduler.hpp"

#include "concurrencpp/coroutines/coroutine.h"
#include "concurrencpp/results/result.h"

#include <bsoncxx/json.hpp>
#include <concurrentqueue.h>
#include <optional>

#include <spdlog/spdlog.h>
#include <spdlog/pattern_formatter.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/fmt/ostr.h>

enum class LogLevel
{
    Trace,
    Debug,
    Info,
    Warn,
    Error,
};

#define HEXIFY(value) StringUtils::ToHex(std::to_wstring(value))

// #define _DEBUG
#ifdef _DEBUG
    #define TRACE(message, ...) spdlog::trace(message "{}", JsonLogFormatter({ __VA_ARGS__ }))
    #define TRACE_W             spdlog::trace(StringUtils::wstos(message).append("{}"), JsonLogFormatter({ __VA_ARGS__ }))
#else
    #define TRACE(message, ...)
    #define TRACE_W
#endif

#define DEBUG(message, ...) spdlog::debug(message "{}", JsonLogFormatter({ __VA_ARGS__ }))
#define INFO(message, ...)  spdlog::info(message "{}", JsonLogFormatter({ __VA_ARGS__ }))
#define WARN(message, ...)  spdlog::warn(message "{}", JsonLogFormatter({ __VA_ARGS__ }))
#define ERROR(message, ...) spdlog::error(message "{}", JsonLogFormatter({ __VA_ARGS__ }))

// ReSharper disable once CppUnnamedNamespaceInHeaderFile
namespace
{
    template <class... Ts>
    struct overloaded : Ts...
    {
            using Ts::operator()...;
    };
    template <class... Ts>
    overloaded(Ts...) -> overloaded<Ts...>;
} // namespace

using namespace std::string_view_literals;

struct JsonLogFormatter
{
        static constexpr std::string_view EoLMarker = "\x18\x19"sv;
        using jsonVal = std::variant<std::int64_t, int, uint, std::uint64_t, DWORD, short, ushort, byte, char, double, float, bool, ClientId, SystemId, BaseId,
                                     ObjectId, RepGroupId, RepId, GoodId, Id, std::string, std::wstring, std::string_view, std::wstring_view, const char*,
                                     const wchar_t*, char*, wchar_t*>;
        std::unordered_map<std::string, jsonVal> members;

        JsonLogFormatter(const std::initializer_list<std::pair<const std::string, jsonVal>> il) : members{ il } {}

        template <typename OStream>
        friend OStream& operator<<(OStream& os, const JsonLogFormatter& j)
        {
            static auto getNickname = [](const uint id)
            {
                auto name = InternalApi::HashLookup(id);
                if (name.empty())
                {
                    return std::format("\"{}\"", std::to_string(id));
                }

                return name;
            };

            os << std::format("{}{{", EoLMarker);
            bool first = true;
            for (const auto& [key, value] : j.members)
            {
                if (!first)
                {
                    os << ",";
                }

                first = false;
                os << std::quoted(key) << ":";
                std::visit(
                    overloaded{ [&](std::int64_t arg) { os << arg; },
                                [&](int arg) { os << arg; },
                                [&](uint arg) { os << arg; },
                                [&](std::uint64_t arg) { os << arg; },
                                [&](DWORD arg) { os << arg; },
                                [&](double arg) { os << arg; },
                                [&](float arg) { os << arg; },
                                [&](short arg) { os << arg; },
                                [&](ushort arg) { os << arg; },
                                [&](byte arg) { os << "\"" << std::hex << "0x" << arg << "\"" << std::dec; },
                                [&](char arg) { os << std::quoted(std::string(1, arg)); },
                                [&](char* arg) { os << std::quoted(arg); },
                                [&](const char* arg) { os << std::quoted(arg); },
                                [&](wchar_t* arg) { os << std::quoted(StringUtils::wstos(arg)); },
                                [&](const wchar_t* arg) { os << std::quoted(StringUtils::wstos(arg)); },
                                [&](const Id arg) { os << getNickname(arg.GetValue()); },
                                [&](const ClientId arg) { os << getNickname(arg.GetValue()); },
                                [&](const SystemId arg) { os << getNickname(arg.GetValue()); },
                                [&](const BaseId arg) { os << getNickname(arg.GetValue()); },
                                [&](const ObjectId arg)
                                { os << arg.GetArchetype().HasValue() ? getNickname(arg.GetArchetype().Value()->archId.GetValue()) : std::string("null"); },
                                [&](const RepGroupId arg) { os << getNickname(arg.GetValue()); },
                                [&](const RepId arg) { os << getNickname(arg.GetValue()); },
                                [&](const GoodId arg) { os << getNickname(arg.GetHash().Unwrap().GetValue()); },
                                [&](const std::string& arg) { os << std::quoted(arg); },
                                [&](const std::string_view arg) { os << std::quoted(arg); },
                                [&](const std::wstring& arg) { os << std::quoted(StringUtils::wstos(arg)); },
                                [&](const std::wstring_view arg) { os << std::quoted(StringUtils::wstos(arg)); },
                                [&](const bool arg) { os << (arg ? "true" : "false"); } },
                    value);
            }
            os << "}";
            return os;
        }
};

template <>
struct fmt::formatter<JsonLogFormatter> : fmt::ostream_formatter
{};

template <typename Mutex>
class DLL MongoSink final : public spdlog::sinks::base_sink<Mutex>
{
        static concurrencpp::result<void> SendToMongo(const spdlog::details::log_msg& msg)
        {
            auto document = bsoncxx::from_json({ msg.payload.data(), msg.payload.size() });
            THREAD_BACKGROUND;

            FLHook::GetDatabase()->BeginDatabaseQuery().InsertIntoCollection(DatabaseCollection::ServerLog, { document });
        }

        void sink_it_(const spdlog::details::log_msg& msg) override { SendToMongo(msg); }
        void flush_() override {}
};

class DLL FlHookLogFormatFlag final : public spdlog::custom_flag_formatter
{
    public:
        void format(const spdlog::details::log_msg& m, const std::tm& tm, spdlog::memory_buf_t& dest) override;
        std::unique_ptr<custom_flag_formatter> clone() const override;
};

class FLHook;
class DLL Logger final
{
        friend FLHook;

        inline static std::shared_ptr<spdlog::logger> logger;
        inline static spdlog::sink_ptr mongoSink;
        inline static spdlog::sink_ptr consoleSink;
        inline static bool consoleAllocated = true;
        inline static HANDLE consoleInput;
        inline static HANDLE consoleOutput;

        inline static std::jthread commandThread;
        inline static moodycamel::ConcurrentQueue<std::wstring> commandQueue; // NOLINT

        static void GetConsoleInput(std::stop_token st);
        static void Init();
        static std::optional<std::wstring> GetCommand();

    public:
        static void MessageConsole(std::wstring_view message);
        static std::shared_ptr<spdlog::logger> GetLogger();
        Logger() = delete;
};
