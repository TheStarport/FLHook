#pragma once

#include <concurrentqueue/concurrentqueue.h>
#include <any>

class DLL TaskScheduler
{
    public:
        struct CallbackTask
        {
            std::variant<std::function<void()>, std::function<bool(std::shared_ptr<void>)>> task;
            std::optional<std::variant<std::function<void(std::shared_ptr<void>)>, std::function<void()>>> callback;
            std::shared_ptr<void> taskData = nullptr;
        };

    private:
        friend FLHook;
        static void ProcessTasks(const std::stop_token& st);
        static std::optional<CallbackTask> GetCompletedTask();

        inline static moodycamel::ConcurrentQueue<CallbackTask> incompleteTasks{};
        inline static moodycamel::ConcurrentQueue<CallbackTask> completeTasks{};

        inline static std::jthread taskProcessorThread{ ProcessTasks };

    public:
        static void Schedule(std::function<void()> task);
        template<typename T>
        static void ScheduleWithCallback(std::function<bool(const std::shared_ptr<void>&)> task, std::function<void(const std::shared_ptr<void>&)> callback)
        {
            if constexpr (!std::is_same_v<void, T>)
            {
                incompleteTasks.enqueue({ task, callback, std::make_shared<T>() });
            }
            else
            {
                incompleteTasks.enqueue({ task, callback, nullptr });
            }
        }

        static void ScheduleWithCallback(std::function<bool()> task, std::function<void()> callback);
};
