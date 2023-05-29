#pragma once

template<class T>
T transform_arg(std::string const& s);
template<>
inline std::string transform_arg(std::string const& s)
{
	return s;
};
template<>
inline double transform_arg(std::string const& s)
{
	return atof(s.c_str());
}
template<>
inline int transform_arg(std::string const& s)
{
	return atoi(s.c_str());
}
template<>
inline float transform_arg(std::string const& s)
{
	return atof(s.c_str());
}

template<typename... Args, std::size_t... Is>
auto create_tuple_impl(std::index_sequence<Is...>, const std::vector<std::string>& arguments)
{
	return std::make_tuple(transform_arg<Args>(arguments[Is])...);
}

template<typename... Args>
auto create_tuple(const std::vector<std::string>& arguments)
{
	return create_tuple_impl<Args...>(std::index_sequence_for<Args...> {}, arguments);
}

template<typename F, F f>
class wrapper;

template<class Ret, class Cl, class... Args, Ret (Cl::*func)(Args...)>
class wrapper<Ret (Cl::*)(Args...), func>
{
  public:
	static Ret ProcessParam(Cl cl, const std::vector<std::string>& params)
	{
		auto arg = create_tuple<Args...>(params);
		auto lambda = std::function<Ret(Args...)> {[=](Args... args) mutable {
			return (cl.*func)(args...);
		}};
		return std::apply(lambda, arg);
	}
};

template<typename T>
struct first_template_type;

template<template<typename T, typename...> class t, typename T, typename... Args>
struct first_template_type<t<T, Args...>>
{
	typedef T type_t;
};