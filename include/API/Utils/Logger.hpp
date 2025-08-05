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

#define LOG_CAT(category)   { "category", category }

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
                                     ShipId, ObjectId, RepGroupId, RepId, GoodId, Id, CharacterId, std::string, std::wstring, std::string_view,
                                     std::wstring_view, const char*, const wchar_t*, char*, wchar_t*, Vector>;
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
                    return std::to_string(id);
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
                    overloaded{
                        [&](std::int64_t arg) { os << arg; },
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
                        [&](const Id arg) { os << std::quoted(getNickname(arg.GetValue())); },
                        [&](const ClientId arg) { os << std::quoted(StringUtils::wstos(arg.GetCharacterId().Unwrap().GetValue())); },
                        [&](const SystemId arg) { os << std::quoted(getNickname(arg.GetValue())); },
                        [&](const BaseId arg) { os << std::quoted(getNickname(arg.GetValue())); },
                        [&](const ShipId arg) { os << std::quoted(StringUtils::wstos(arg.GetNickName().Unwrap())); },
                        [&](const ObjectId arg)
                        {
                            os << std::quoted(arg.GetArchetype().HasValue() ? getNickname(arg.GetArchetype().Value()->archId.GetValue()) : std::string("null"));
                        },
                        [&](const RepGroupId arg) { os << std::quoted(getNickname(arg.GetValue())); },
                        [&](const RepId arg) { os << std::quoted(getNickname(arg.GetValue())); },
                        [&](const GoodId arg) { os << std::quoted(getNickname(arg.GetHash().Unwrap().GetValue())); },
                        [&](const CharacterId& arg) { os << std::quoted(StringUtils::wstos(arg.GetValue())); },
                        [&](const std::string& arg) { os << std::quoted(arg); },
                        [&](const std::string_view arg) { os << std::quoted(arg); },
                        [&](const std::wstring& arg) { os << std::quoted(StringUtils::wstos(arg)); },
                        [&](const std::wstring_view arg) { os << std::quoted(StringUtils::wstos(arg)); },
                        [&](const bool arg) { os << (arg ? "true" : "false"); },
                        [&](const Vector arg) { os << std::setprecision(0) << "[" << arg.x << "," << arg.y << "," << arg.z << "]" << std::setprecision(6); },
                    },
                    value);
            }
            os << "}";
            return os;
        }
};

template <>
struct fmt::formatter<JsonLogFormatter> : fmt::ostream_formatter
{};

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
