#pragma once
#include <memory>

template<typename T>
class Singleton
{
  public:
	static T* i(std::unique_ptr<T>* o = nullptr)
  	{
		static std::unique_ptr<T> i = std::unique_ptr<T>(new T());
		if (o)
		{
			i = std::move(*o);
		}
		return i.get();
	}
	static T& ir(std::unique_ptr<T>* o = nullptr) { return *i(o); }

	static const T* c() { return i(); }
};