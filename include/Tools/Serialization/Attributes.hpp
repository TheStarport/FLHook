#pragma once

#include <refl.hpp>

template<typename T>
    requires IsNumeric<T>
struct AttrMin : refl::attr::usage::field
{
	const T val;
	explicit constexpr AttrMin(T v) noexcept : val(v)
	{
		// no impl
	}

	cpp::result<void, std::string> Validate(const T& comp) const
	{
		if (comp >= val)
		{
			return {};
		}
		return cpp::fail(std::string("Value was below minimum threshold"));
	}
};

REFL_AUTO(template((typename T), (AttrMin<T>)), func(Validate));

template<typename T>
    requires IsNumeric<T>
struct AttrMax : refl::attr::usage::field
{
	const T val;
	
	explicit constexpr AttrMax(T v) noexcept : val(v)
	{
		// no impl
	}

	cpp::result<void, std::string> Validate(const T& comp) const 
	{ 
		if (comp <= val)
		{
			return {};
		}
		return cpp::fail(std::string("Value was above maximum threshold"));
	}
};

REFL_AUTO(template((typename T), (AttrMax<T>)), func(Validate));

template<typename T>
    requires StringRestriction<T>
struct AttrNotEmptyNotWhiteSpace : refl::attr::usage::field
{
	explicit constexpr AttrNotEmptyNotWhiteSpace() noexcept
	{
		// no impl
	}

	cpp::result<void, std::string> Validate(const T& comp) const
	{
		if (comp.empty() || std::ranges::all_of(comp, [](const auto& c) { return c == ' ' || c == '\n' || c == '\r' || c == '\t'; }))
		{
			return cpp::fail(std::string("Value was empty"));
		}
		return {};
	}
};

REFL_AUTO(template((typename T), (AttrNotEmptyNotWhiteSpace<T>)), func(Validate));

template<typename T>
    requires StringRestriction<T>
struct AttrMaxLength : refl::attr::usage::field
{
	const uint val;
	explicit constexpr AttrMaxLength(uint v) noexcept : val(v)
	{
		// no impl
	}

	cpp::result<void, std::string> Validate(const T& comp) const
	{
		if (comp.length() > val)
		{
			return cpp::fail(std::format("Value {} was above max of {}", comp.length(), val));
		}
		return {};
	}
};

REFL_AUTO(template((typename T), (AttrMaxLength<T>)), func(Validate));

template<typename T>
    requires StringRestriction<T>
struct AttrMinLength : refl::attr::usage::field
{
	const uint val;
	explicit constexpr AttrMinLength(uint v) noexcept : val(v)
	{
		// no impl
	}

	cpp::result<void, std::string> Validate(const T& comp) const
	{
		if (comp.length() < val)
		{
			return cpp::fail(std::format("Value {} was below max of {}", comp.length(), val));
		}
		return {};
	}
};

REFL_AUTO(template((typename T), (AttrMinLength<T>)), func(Validate));


template<typename T>
using AttrLambda = cpp::result<void, std::string>(*)(T);

template<typename T, typename TT>
concept LambdaConstraint = std::is_same_v<T, AttrLambda<TT>>;

template<typename T, typename Lambda>
struct AttrCustom : refl::attr::usage::field
{
	Lambda func;
	explicit constexpr AttrCustom(const Lambda& l) noexcept : func(l)
	{
		// no impl
	}

	cpp::result<void, std::string> Validate(const T& comp) const { return func(comp); }
};

REFL_AUTO(template((typename T, typename Lambda), (AttrCustom<T, Lambda>)), func(Validate));

template<typename T>
struct AttrNotEmpty : refl::attr::usage::field
{
	explicit constexpr AttrNotEmpty() noexcept
	{
		// no impl
	}

	cpp::result<void, std::string> Validate(const T& comp) const
	{
		if (comp.empty())
		{
			return cpp::fail(std::string("Value was empty"));
		}
		return {};
	}
};

REFL_AUTO(template((typename T), (AttrNotEmpty<T>)), func(Validate));