#pragma once

#include "GameException.hpp"

template<typename Ret>
    requires RequireParamterlessConstructor<Ret>
class Action<Ret>
{
	cpp::result<Ret, Error> result;

  public:
	explicit Action(cpp::result<Ret, Error> res) : result(res) {}

	Ret Handle(const std::optional<std::function<void(Error, std::wstring_view)>>& func = std::nullopt)
	{
		if (result.has_error())
		{
			if (func == std::nullopt)
			{
				throw GameException(L"An error has occurred.", result.error());
			}

			func(result.error(), L"An error has occurred.");
			return Ret();
		}

		return result.value();
	}

	Ret Unwrap() { return result.value_or(Ret()); }

	const cpp::result<Ret, Error>& Raw() { return result; }
};
