#pragma once

#include <any>
#include <nlohmann/json_fwd.hpp>


using namespace std::string_view_literals;
class AdminCommandProcessor
{
	template<class T>
	T transform_arg(std::string const& s);
	template<>
	static std::string transform_arg(std::string const& s)
	{
		return s;
	};
	template<>
	static double transform_arg(std::string const& s)
	{
		return atof(s.c_str());
	}
	template<>
	static int transform_arg(std::string const& s)
	{
		return atoi(s.c_str());
	}
	template<>
	static float transform_arg(std::string const& s)
	{
		return atof(s.c_str());
	}

	template<typename... Args, std::size_t... Is>
	static auto create_tuple_impl(std::index_sequence<Is...>, const std::vector<std::string>& arguments)
	{
		return std::make_tuple(transform_arg<Args>(arguments[Is])...);
	}

	template<typename... Args>
	static auto create_tuple(const std::vector<std::string>& arguments)
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

	constexpr static std::array<std::string_view, 2> commands = {
	    "GetCash"sv,
	    "SetCash"sv,
	};

	const std::array<cpp::result<nlohmann::json, nlohmann::json> (*)(AdminCommandProcessor cl, const std::vector<std::string>& params),
	    2 > funcs = {
	    wrapper<decltype(&AdminCommandProcessor::GetCash), &AdminCommandProcessor::GetCash>::ProcessParam,
	    wrapper<decltype(&AdminCommandProcessor::SetCash), &AdminCommandProcessor::SetCash>::ProcessParam};

	cpp::result<nlohmann::json, nlohmann::json> SetCash(std::string_view characterName, uint amount);
	cpp::result<nlohmann::json, nlohmann::json> GetCash(std::string_view characterName);


  public:
	template<int N>
	void MatchCommand(AdminCommandProcessor* processor, std::string_view cmd, const std::vector<std::string> paramVector)
	{
		const auto command = std::get<N - 1>(processor->commands);
		const auto func = std::get<N - 1>(processor->funcs);
		if (command == cmd)
		{
			func(*processor, paramVector);
			return;
		}
		MatchCommand<N - 1>(processor, cmd, paramVector);
	}

	template<>
	void MatchCommand<0>(AdminCommandProcessor* processor, std::string_view cmd, const std::vector<std::string> paramVector)
	{
	}

	void ProcessCommand(std::string_view command);
};