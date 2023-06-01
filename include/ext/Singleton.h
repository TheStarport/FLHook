#pragma once
#include <memory>

template<typename T>
class Singleton
{
  public:
	static T* i(std::unique_ptr<T>* o = nullptr, bool reset = false)
  	{
		static std::unique_ptr<T> i = std::unique_ptr<T>(new T());
		if (reset)
		{
			// Explicitly delete and release
			i.reset();
			return nullptr;
		}
		if (o)
		{
			i = std::move(*o);
		}
		else if (!i)
		{
			i = std::unique_ptr<T>(new T());
		}
		return i.get();
	}

	static const T* c() { return i(); }
	static void del() { i(nullptr, true); }
};