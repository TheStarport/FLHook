#pragma once

#include <concurrentqueue/concurrentqueue.h>
#include <any>

class DLL TaskScheduler
{
    public:
        struct CallbackTask
        {
            std::variant<std::function<void()>, std::function<bool(std::any)>> task;
            std::optional<std::variant<std::function<void(std::any)>, std::function<void()>>> callback;
            byte* taskData = nullptr;
            size_t dataSize;
        };

    private:
        friend FLHook;
        inline static std::allocator<byte> allocator;
        static void ProcessTasks(const std::stop_token& st);
        static std::optional<CallbackTask> GetCompletedTask();

        inline static moodycamel::ConcurrentQueue<CallbackTask> incompleteTasks;
        inline static moodycamel::ConcurrentQueue<CallbackTask> completeTasks;

        inline static std::jthread taskProcessorThread{ ProcessTasks };

    public:
        static void Schedule(std::function<void()> task);
        template<typename T>
        static void ScheduleWithCallback(std::function<bool(std::any)> task, std::function<void(std::any)> callback)
        {
            if constexpr (!std::is_same_v<void, T>)
            {
                const auto data = allocator.allocate(sizeof(T));
                incompleteTasks.enqueue({ task, callback, data, sizeof(T) });
            }
            else
            {
                incompleteTasks.enqueue({ task, callback, nullptr, 0 });
            }
        }

        static void ScheduleWithCallback(std::function<bool()> task, std::function<void()> callback);
};
