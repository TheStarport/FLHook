#include "PCH.hpp"

#include "API/FLHook/TaskScheduler.hpp"

void TaskScheduler::ProcessTasks(const std::stop_token& st)
{
    using namespace std::chrono_literals;

    while (!st.stop_requested())
    {
        Task task;
        while (incompleteTasks.try_dequeue(task))
        {
            task.task();
            if (task.callback.has_value())
            {
                completeTasks.enqueue(task);
            }
        }

        std::this_thread::sleep_for(50ms);
    }
}

void TaskScheduler::Schedule(Func task) { incompleteTasks.enqueue({ task }); }

void TaskScheduler::ScheduleWithCallback(Func task, Func callback) { incompleteTasks.enqueue({ task, callback }); }

std::optional<TaskScheduler::Task> TaskScheduler::GetCompletedTask()
{
    Task task;
    if (!completeTasks.try_dequeue(task))
    {
        return {};
    }

    return task;
}
