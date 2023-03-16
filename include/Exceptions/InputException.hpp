#pragma once

#include "ErrorInfo.hpp"

class DLL InputException final : std::exception
{
	Error err;

public:
	explicit InputException(const Error err) : err(err) {}
	~InputException() noexcept override = default;
	[[nodiscard]] char const* what() const override
	{
		return ErrorInfo::i()->GetText(err).c_str();
	}
};