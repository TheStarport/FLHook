// ReSharper disable CppMemberFunctionMayBeStatic
#include "PCH.hpp"

#include "API/FLHook/TaskScheduler.hpp"

TaskScheduler::TaskScheduler()
{
    executor = runtime.make_manual_executor();
    backgroundExecutor = runtime.thread_pool_executor();
    timerQueue = runtime.timer_queue();
}

void TaskScheduler::ProcessTasks()
{
    const auto msNow = TimeUtils::UnixTime<std::chrono::milliseconds>();
    bool processed = false;
    do
    {
        processed = executor->loop_once();
    }
    while (processed && msNow + 12 > TimeUtils::UnixTime<std::chrono::milliseconds>());

    for (auto iter = taskHandles.begin(); iter != taskHandles.end();)
    {
        const auto& task = *iter;
        assert(task);

        if (!task->result)
        {
            iter = taskHandles.erase(iter);
            continue;
        }

        if (task->result.status() == concurrencpp::result_status::exception)
        {
            // ReSharper disable once CppPassValueParameterByConstReference
            static auto informUser = [](const ClientId client, const std::wstring message) -> concurrencpp::result<void>
            {
                client.MessageErr(message);
                co_return;
            };

            try
            {
                // Intentionally trigger the exception to handle gracefully
                task->result.get();
            }
            catch (const InvalidParameterException& ex)
            {
                auto client = task->client;
                executor->submit(informUser, client, std::wstring(ex.Msg()));
            }
            catch (GameException& ex)
            {
                auto client = task->client;
                executor->submit(informUser, client, std::wstring(ex.Msg()));
            }
            catch (const StopProcessingException&)
            {
                //
            }
            catch (const std::exception& ex)
            {
                // Anything else critically log
                ERROR("Exception thrown during coroutine: {{ex}}", { "ex", ex.what() });
            }
        }

        ++iter;
    }
}

concurrencpp::details::resume_on_awaitable<concurrencpp::thread_pool_executor> TaskScheduler::RunOnBackgroundThread() const
{
    return concurrencpp::resume_on(backgroundExecutor);
}

concurrencpp::details::resume_on_awaitable<concurrencpp::manual_executor> TaskScheduler::RunOnMainThread() const { return concurrencpp::resume_on(executor); }

Task::Task(concurrencpp::result<void> result, const ClientId client) : result(std::move(result)), client(client) {}
concurrencpp::result<void> Task::NullOp() { return concurrencpp::make_ready_result<void>(); }
