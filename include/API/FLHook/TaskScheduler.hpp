#pragma once

#include <any>
#include <concurrentqueue.h>
#include <coroutine>

enum class TaskStatus
{
    FLHookAwait,
    DatabaseAwait,
    Finished,
    Error,
    Init
};

class TaskScheduler;
class Promise;
class DLL Task
{
        friend TaskScheduler;
        TaskStatus status = TaskStatus::Init;
        std::coroutine_handle<Promise> handle;
        void Run();

    public:
        using promise_type = Promise;

        explicit Task(std::coroutine_handle<Promise> h);
        TaskStatus UpdateStatus();
};

class DLL Promise
{
        TaskStatus status = TaskStatus::Init;

    public:
        [[nodiscard]]
        TaskStatus GetStatus() const;

        // ReSharper disable twice CppMemberFunctionMayBeStatic
        [[nodiscard]]
        std::suspend_never initial_suspend() const noexcept;

        [[nodiscard]]
        std::suspend_always final_suspend() const noexcept;
        Task get_return_object();
        void return_value(TaskStatus &&newStatus);
        std::suspend_always yield_value(TaskStatus &&newStatus);
        void unhandled_exception() noexcept;
};

// TODO: Remove all references to the old callback style of task processing
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
        static void ProcessTasksOld(const std::stop_token &st);
        static std::optional<CallbackTask> GetCompletedTask();

        inline static moodycamel::ConcurrentQueue<CallbackTask> incompleteTasksOld{};
        inline static moodycamel::ConcurrentQueue<CallbackTask> completeTasksOld{};

        void ProcessDatabaseTasks(const std::stop_token &st);
        void ProcessTasks(moodycamel::ConcurrentQueue<std::shared_ptr<Task>> &tasks);
        moodycamel::ConcurrentQueue<std::shared_ptr<Task>> mainTasks{};
        moodycamel::ConcurrentQueue<std::shared_ptr<Task>> databaseTasks{};

        std::jthread databaseThread;

        inline static std::jthread taskProcessorThread{ ProcessTasksOld };

    public:
        TaskScheduler() : databaseThread(std::bind_front(&TaskScheduler::ProcessDatabaseTasks, this)) {}
        ~TaskScheduler() = default;

        static void Schedule(std::function<void()> task);
        template <typename T>
        static void ScheduleWithCallback(std::function<bool(const std::shared_ptr<void> &)> task, std::function<void(const std::shared_ptr<void> &)> callback)
        {
            if constexpr (!std::is_same_v<void, T>)
            {
                incompleteTasksOld.enqueue({ task, callback, std::make_shared<T>() });
            }
            else
            {
                incompleteTasksOld.enqueue({ task, callback, nullptr });
            }
        }

        static void ScheduleWithCallback(std::function<bool()> task, std::function<void()> callback);
        bool AddTask(const std::shared_ptr<Task> &task);
};
