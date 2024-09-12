// ReSharper disable CppMemberFunctionMayBeStatic
#include "PCH.hpp"

#include "API/FLHook/TaskScheduler.hpp"

Task::Task(const std::coroutine_handle<Promise> h) : handle(h) {}

void Task::Run()
{
    if (status == TaskStatus::Finished)
    {
        handle.destroy();
        return;
    }

    handle.resume();
    UpdateStatus();
}

TaskStatus Task::UpdateStatus()
{
    status = handle.promise().GetStatus();
    return status;
}

TaskStatus Promise::GetStatus() const { return status; }
std::suspend_never Promise::initial_suspend() const noexcept { return {}; }
std::suspend_always Promise::final_suspend() const noexcept { return {}; }
Task Promise::get_return_object() { return Task(std::coroutine_handle<Promise>::from_promise(*this)); }
void Promise::return_value(TaskStatus&& newStatus) { status = std::forward<TaskStatus>(newStatus); }

std::suspend_always Promise::yield_value(TaskStatus&& newStatus)
{
    status = std::forward<TaskStatus>(newStatus);
    return {};
}

void Promise::unhandled_exception() noexcept
{

    constexpr std::wstring_view err = L"Unhandled exception occured while handling Promise";
    Logger::Err(err);
}

void TaskScheduler::ProcessTasksOld(const std::stop_token& st)
{
    using namespace std::chrono_literals;

    while (!st.stop_requested())
    {
        CallbackTask task;
        while (incompleteTasksOld.try_dequeue(task))
        {
            if (task.task.index() == 0)
            {
                std::get<0>(task.task)();
                continue;
            }

            if (const auto invokeCallback = std::get<1>(task.task)(task.taskData); invokeCallback && task.callback.has_value())
            {
                completeTasksOld.enqueue(task);
            }
        }

        std::this_thread::sleep_for(50ms);
    }
}

void TaskScheduler::Schedule(std::function<void()> task) { incompleteTasksOld.enqueue({ task }); }

void TaskScheduler::ScheduleWithCallback(std::function<bool()> task, std::function<void()> callback)
{
    incompleteTasksOld.enqueue(CallbackTask{ task, callback, nullptr });
}

bool TaskScheduler::AddTask(const std::shared_ptr<Task>& task)
{
    task->UpdateStatus();
    if (task->status == TaskStatus::Finished)
    {
        return true;
    }

    if (task->status == TaskStatus::FLHookAwait)
    {
        return mainTasks.try_enqueue(task);
    }

    if (task->status == TaskStatus::DatabaseAwait)
    {
        return databaseTasks.try_enqueue(task);
    }

    return false;
}

std::optional<TaskScheduler::CallbackTask> TaskScheduler::GetCompletedTask()
{
    CallbackTask task;
    if (!completeTasksOld.try_dequeue(task))
    {
        return {};
    }

    return task;
}

void TaskScheduler::ProcessDatabaseTasks(const std::stop_token& st)
{
    using namespace std::chrono_literals;
    while (!st.stop_requested())
    {
        std::this_thread::sleep_for(1ms);

        ProcessTasks(databaseTasks);
    }
}

void TaskScheduler::ProcessTasks(moodycamel::ConcurrentQueue<std::shared_ptr<Task>>& tasks)
{
    for (size_t i = tasks.size_approx(); i > 0; i--)
    {
        std::shared_ptr<Task> t;
        if (!tasks.try_dequeue(t))
        {
            break;
        }

        Logger::Info(magic_enum::enum_name(t->status));
        t->Run();
        Logger::Info(magic_enum::enum_name(t->status));

        if (t->status == TaskStatus::FLHookAwait)
        {
            mainTasks.try_enqueue(t);
        }
        else if (t->status == TaskStatus::DatabaseAwait)
        {
            databaseTasks.try_enqueue(t);
        }
    }
}
