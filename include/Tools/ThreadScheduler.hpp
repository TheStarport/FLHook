#pragma once
#include "concurrencpp/concurrencpp.h"

class ThreadScheduler : public Singleton<ThreadScheduler>
{
	concurrencpp::runtime runtime;
	std::shared_ptr<concurrencpp::thread_pool_executor> executor;

	public: 
		ThreadScheduler();
};