#pragma once
#include "MyCppForward.h"

class ThreadTaskQueue;

MYCPP_API extern std::unordered_map<std::thread::id, ThreadTaskQueue*> G_ThreadIdToTaskQueueMap;
MYCPP_API ThreadTaskQueue& CurrentThreadTaskQueueRef();

class MYCPP_API ThreadTaskQueue
{
public:
    void EnqueueTask(const std::function<void()>& task)
    {
        _TaskQueue.Enqueue(task);
    }

    void EnqueueTask(std::function<void()>&& task)
    {
        _TaskQueue.Enqueue(std::move(task));
    }

    uint32_t ProcessTask()
    {
        std::function<void()> task;
        uint32_t counter = 0;
        while (_TaskQueue.Dequeue(task))
        {
            task();
            counter++;
        }
        return counter;
    }

public:
    TQueue<std::function<void()>> _TaskQueue;
    std::thread::id _ThreadId;
};

template<typename Function, typename ... FunctionArgs>
bool PostTask(std::thread::id thread_id, Function&& task, FunctionArgs&& ... args)
{
    auto thread_task_queue_iterator = G_ThreadIdToTaskQueueMap.find(thread_id);
    if (thread_task_queue_iterator != G_ThreadIdToTaskQueueMap.end())
    {
        thread_task_queue_iterator->second->EnqueueTask(std::bind(std::forward<Function>(task), std::forward<FunctionArgs>(task)...));
        return true;
    }
    return false;
}

















