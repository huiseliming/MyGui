#include "ThreadTaskQueue.h"

std::unordered_map<std::thread::id, ThreadTaskQueue*> G_ThreadIdToTaskQueueMap;

static std::unique_ptr<ThreadTaskQueue> 
NewThreadTaskQueue () 
{
    ThreadTaskQueue* thread_task_queue = new ThreadTaskQueue;
    G_ThreadIdToTaskQueueMap.insert(std::pair(std::this_thread::get_id(), thread_task_queue));
    return std::unique_ptr<ThreadTaskQueue>(thread_task_queue);
}

ThreadTaskQueue&
CurrentThreadTaskQueueRef()
{
    static thread_local std::unique_ptr<ThreadTaskQueue> TLS_ThreadTaskQueue = NewThreadTaskQueue();
    return *TLS_ThreadTaskQueue;
}





















