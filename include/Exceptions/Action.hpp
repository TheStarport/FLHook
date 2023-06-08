#pragma once

#include "GameException.hpp"

template<typename Ret>
    requires RequireParamterlessConstructor<Ret>
class Action<Ret>
{
	cpp::result<Ret, Error> result;

  public:
	Action(cpp::result<Ret, Error> res) : result(res) {}

	//Will check to see if the function has returned an error and throws a GameException if so, you may handle the error yourself by
	// passing in a lambda in the arguments or let Flhook's exception handler do it for you by passing in no arguments.
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
	//Simple returns the result value without the error, will return a default value if an error is present.
	Ret Unwrap() { return result.value_or(Ret()); }

	//Returns a cpp::result of the return type and Error.
	const cpp::result<Ret, Error>& Raw() { return result; }
};
