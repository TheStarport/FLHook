#include "PCH.hpp"

#include "Exceptions/GameException.hpp"

#include <Utils/StringUtils.hpp>
#include <cpptrace/basic.hpp>

GameException::GameException(const std::wstring& msg, const Error err) : msg(std::format(L"{}\n{}", msg, ErrorInfo::GetText(err)))
{
    trace = StringUtils::stows(cpptrace::generate_trace().to_string());
}

GameException::GameException(const std::wstring& msg) : msg(msg) { trace = StringUtils::stows(cpptrace::generate_trace().to_string()); }
