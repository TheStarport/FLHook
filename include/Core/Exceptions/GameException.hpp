#pragma once

#include "ErrorInfo.hpp"
#include "UnsupportedException.hpp"

class DLL GameException final : std::exception
{
	std::wstring msg;

  public:
	explicit GameException(std::wstring msg, const Error err) : msg(std::format(L"{}\n{}", std::move(msg), ErrorInfo::GetText(err))) {}
	~GameException() noexcept override = default;

	/**
	 * @deprecated Using the const char* what() on custom exceptions is not supported.
	 */
	[[nodiscard]] char const* what() const override { throw UnsupportedException(); }

	/**
	 * @brief Retrieve the underlying error message from the exception
	 */
	std::wstring_view Msg() const { return msg; }
};