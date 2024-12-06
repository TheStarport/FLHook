// ReSharper disable CppMemberFunctionMayBeStatic
#include "PCH.hpp"

#include "API/FLHook/TaskScheduler.hpp"

#include "Exceptions/InvalidParameterException.hpp"

Task::Task(const std::coroutine_handle<Promise> h) : handle(h) {}
void Task::SetClient(ClientId c) { client = c; }
bool Task::HandleException()
{
    UpdateStatus();

    if (status == TaskStatus::Finished)
    {
        return true;
    }

    if (const auto exception = handle.promise().GetException(); exception != nullptr)
    {
        try
        {
            std::rethrow_exception(exception);
        }
        catch (const StopProcessingException&)
        {}
        catch (const GameException& ex)
        {
            if (client.has_value())
            {
                client->Message(ex.Msg());
            }
            else
            {
                Logger::Warn(ex.Msg());
            }
        }
        catch (const std::exception& ex)
        {
            Logger::Err(std::format(L"Uncaught exception thrown during Task processing!\n{}", StringUtils::stows(ex.what())));
        }

        // Terminate the task early due to the exception
        exceptioned = true;
        status = TaskStatus::Finished;
        return false;
    }

    return true;
}

void Task::Run()
{
    if (status == TaskStatus::Finished)
    {
        handle.destroy();
        return;
    }

    handle.resume();
    if (!HandleException())
    {
        return;
    }

    UpdateStatus();
}

TaskStatus Task::UpdateStatus()
{
    if (!exceptioned)
    {
        status = handle.promise().GetStatus();
    }

    return status;
}

TaskStatus Promise::GetStatus() const { return status; }
std::exception_ptr Promise::GetException()
{
    auto ex = exception;
    exception = nullptr;
    return ex;
}
std::suspend_never Promise::initial_suspend() const noexcept { return {}; }
std::suspend_always Promise::final_suspend() const noexcept { return {}; }
Task Promise::get_return_object() { return Task(std::coroutine_handle<Promise>::from_promise(*this)); }
void Promise::return_value(TaskStatus&& newStatus) { status = std::forward<TaskStatus>(newStatus); }

std::suspend_always Promise::yield_value(TaskStatus&& newStatus)
{
    status = std::forward<TaskStatus>(newStatus);
    return {};
}

void Promise::unhandled_exception() noexcept { exception = std::current_exception(); }

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

        const auto existingStatus = t->status;
        auto informUser = [&t](const std::wstring message) -> Task // NOLINT(*-unnecessary-value-param)
        {
            if (t->client.has_value())
            {
                t->client->MessageErr(message);
            }
            else
            {
                Logger::Warn(message);
            }

            co_return TaskStatus::Finished;
        };

        try
        {
            t->Run();
        }
        catch (InvalidParameterException& ex)
        {
            if (existingStatus == TaskStatus::FLHookAwait)
            {
                informUser(std::wstring(ex.Msg()));
            }
            else
            {
                mainTasks.enqueue(std::make_shared<Task>(informUser(std::wstring(ex.Msg()))));
            }
        }
        catch (GameException& ex)
        {
            if (existingStatus == TaskStatus::FLHookAwait)
            {
                informUser(std::wstring(ex.Msg()));
            }
            else
            {
                mainTasks.enqueue(std::make_shared<Task>(informUser(std::wstring(ex.Msg()))));
            }
        }
        catch (StopProcessingException&)
        {
            // Continue processing
        }
        catch (std::exception& ex)
        {
            // Anything else critically log
            Logger::Err(StringUtils::stows(ex.what()));
        }

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
