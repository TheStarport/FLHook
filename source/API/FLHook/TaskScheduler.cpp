#include "PCH.hpp"

#include "API/FLHook/TaskScheduler.hpp"

void TaskScheduler::ProcessTasks(const std::stop_token& st)
{
    using namespace std::chrono_literals;

    while (!st.stop_requested())
    {
        CallbackTask task;
        while (incompleteTasks.try_dequeue(task))
        {
            if (task.task.index() == 0)
            {
                std::get<0>(task.task)();
            }
            else
            {
                if (const auto invokeCallback = std::get<1>(task.task)(task.taskData); !invokeCallback)
                {
                    if (task.taskData)
                    {
                        allocator.deallocate(task.taskData, task.dataSize);
                    }
                }
                else if (task.callback.has_value())
                {
                    completeTasks.enqueue(task);
                }
            }

        }

        std::this_thread::sleep_for(50ms);
    }
}

void TaskScheduler::Schedule(std::function<void()> task) { incompleteTasks.enqueue({ task }); }

void TaskScheduler::ScheduleWithCallback(std::function<bool()> task, std::function<void()> callback)
{
    incompleteTasks.enqueue(CallbackTask{ task, callback, nullptr, 0 });
}

std::optional<TaskScheduler::CallbackTask> TaskScheduler::GetCompletedTask()
{
    CallbackTask task;
    if (!completeTasks.try_dequeue(task))
    {
        return {};
    }

    return task;
}
