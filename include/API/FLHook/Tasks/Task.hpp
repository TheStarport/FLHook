#pragma once
#include <coroutine>
#include <filesystem>

enum class TaskStatus : uint8_t
{
    FLHookAwait,
    DatabaseAwait,
    Finished,
    Error
};

struct Task
{
        struct Promise
        {
                concurrencpp::result<void>get_return_object() { return {}; }
                std::suspend_never initial_suspend() { return {}; }
                std::suspend_always final_suspend() { return {}; }
                void return_value(TaskStatus&& newStatus) { status = std::forward<TaskStatus>(newStatus); }
                std::suspend_always yield_value();

                private : TaskStatus status = TaskStatus::Error;
                std::exception_ptr exception;
        };

        using promise_type = Promise;
};
