#pragma once

#include "Exceptions/InvalidParameterException.hpp"

#include <concurrencpp/concurrencpp.h>

#define THREAD_MAIN       co_await FLHook::GetTaskScheduler()->RunOnMainThread()
#define THREAD_BACKGROUND co_await FLHook::GetTaskScheduler()->RunOnBackgroundThread()

class TaskScheduler;
class DLL Task
{
        friend TaskScheduler;

        ClientId client;
        concurrencpp::result<void> result;

    public:
        explicit Task(concurrencpp::result<void> result, ClientId client);
        static concurrencpp::result<void> NullOp();
};

class FLHook;
class DLL TaskScheduler
{
        friend FLHook;

        concurrencpp::runtime runtime;
        std::shared_ptr<concurrencpp::manual_executor> executor;
        std::shared_ptr<concurrencpp::thread_pool_executor> backgroundExecutor;
        std::shared_ptr<concurrencpp::timer_queue> timerQueue;
        std::list<std::shared_ptr<Task>> taskHandles;

        void ProcessTasks();

    public:
        TaskScheduler();

        template <typename T, class... Args>
        auto ScheduleTask(T callable, Args&&... args)
        {
            return backgroundExecutor->submit(std::forward<T>(callable), std::forward<Args&>(args)...);
        }

        void StoreTaskHandle(const std::shared_ptr<Task>& task) { taskHandles.emplace_back(task); }

        concurrencpp::details::resume_on_awaitable<concurrencpp::thread_pool_executor> RunOnBackgroundThread() const;
        concurrencpp::details::resume_on_awaitable<concurrencpp::manual_executor> RunOnMainThread() const;

        template <typename Duration>
            requires IsChronoDurationV<Duration>
        auto Delay(Duration duration, const bool background = false) const
        {
            auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
            if (background)
            {
                return timerQueue->make_delay_object(durationMs, backgroundExecutor);
            }

            return timerQueue->make_delay_object(durationMs, executor);
        }
};
