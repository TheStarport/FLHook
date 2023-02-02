#pragma once
#include <memory>

template<typename T>
class Singleton
{
  public:
	static T* i(std::unique_ptr<T>* o = nullptr)
	{
		static std::unique_ptr<T> i = o ? std::move(*o) : std::unique_ptr<T>(new T());
		return i.get();
	}
	static T& ir(std::unique_ptr<T>* o = nullptr) { return *i(o); }

	static const T* c() { return i(); }
};