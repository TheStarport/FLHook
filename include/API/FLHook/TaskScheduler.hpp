#pragma once

#include <concurrentqueue/concurrentqueue.h>

class TaskScheduler
{
        static void ProcessTasks(const std::stop_token& st);

    public:
        using Func = std::function<void()>;
        struct Task
        {
                Func task;
                std::optional<Func> callback;
        };

    private:
        inline static moodycamel::ConcurrentQueue<Task> incompleteTasks;
        inline static moodycamel::ConcurrentQueue<Task> completeTasks;

        inline static std::jthread taskProcessorThread{ ProcessTasks };

    public:
        static void Schedule(Func task);
        static void ScheduleWithCallback(Func task, Func callback);
        static std::optional<Task> GetCompletedTask();
};
