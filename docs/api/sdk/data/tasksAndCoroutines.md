---
author: "Nen"
date: "2024-09-01"
title: Asynchronous Commands
---

# Coroutines

As of 5.0 FLHook supports the ability to do computations asynchronously off the main FLServer thread. However there are limitations in its usage. Anything that directly interfaces with Freelancer is unable to be done off the main thread. Thus the main purpose of this feature is to offload IO and Database calls. All commands support this function by default as they must return a Task object which is the FLHook implementation of the coroutine.

Example:
```
Task UserCommandProcessor::GiveCash(clientId client, std::wstring_view characterName, std::wstring_view amount)
{
    ...stuff

    co_return TaskStatus::FINISHED;
}
```

For the majority of cases the only thing you really need to know is specify the return type as Task, and use "co_return TaskStatus::FINISHED" instead of return compared to a normal function. 
I would reccomend not going beyond this if you are unsure of the ideas and concepts behind coroutines and asynchronous computing as misuse of this can create massive performance hits for the server and potential crashes/ unpredictable behavior. 

# When to use them.

The primary use case that will come up with coroutines is when you want to either update or access information from the Database and do some processing after the fact. The way to accomplish it with commands is as thus:

Example:
```
Task AdminCommandProcessor::SetCash(clientId,std::wstring_view characterName, std::wstring_view amount)
{
   co_yield TaskStatus::DATABASE_AWAIT;

   ....database calls.

   co_yield TaskStatus::FLHOOK_AWAIT;

   ...FLHook functions such as messaging the admin that the task was performmed successfully. 

    co_return TaskStatus::FINISHED;
}
```

# Tasks for non commands.

Tasks can also be used for functions that are not part of a command simple create a function like above and call 
```
FLHook::GetTaskScheduler().AddTask(std::make_shared<Task>(YourFunc()));
```