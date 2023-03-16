#pragma once

namespace Hk::Time
{
	DLL uint GetUnixSeconds();
	DLL uint GetUnixMiliseconds();

	template<typename T>
	std::chrono::microseconds ToMicroseconds(T duration)
	{
		return std::chrono::duration_cast<std::chrono::microseconds>(duration);
	}

	template<typename T>
	std::chrono::milliseconds ToMilliseconds(T duration)
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(duration);
	}

	template<typename T>
	std::chrono::seconds ToSeconds(T duration)
	{
		return std::chrono::duration_cast<std::chrono::seconds>(duration);
	}

	template<typename T>
	std::chrono::minutes ToMinutes(T duration)
	{
		return std::chrono::duration_cast<std::chrono::minutes>(duration);
	}

	template<typename T>
	std::chrono::hours ToHours(T duration)
	{
		return std::chrono::duration_cast<std::chrono::hours>(duration);
	}

	template<typename T>
	std::chrono::nanoseconds ToNanoseconds(T duration)
	{
		return std::chrono::duration_cast<std::chrono::nanoseconds>(duration);
	}
}