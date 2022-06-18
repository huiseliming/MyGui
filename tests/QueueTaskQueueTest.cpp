#include <catch2/catch.hpp>
#include <thread>
#include "EasyGui.h"

TEST_CASE("ThreadTaskQueue")
{
    std::thread::id test_thread_id = std::this_thread::get_id();
    std::thread work_thread([] {
        while (CurrentThreadTaskQueueRef().ProcessTask() == 0)
        {
            std::this_thread::yield();
        }
        });
    CurrentThreadTaskQueueRef().ProcessTask();
    std::thread::id work_thread_id = work_thread.get_id();
    while (
        PostTask(work_thread_id, [test_thread_id] {
            PostTask(test_thread_id, [test_thread_id] {
                REQUIRE(test_thread_id == std::this_thread::get_id());
                });
            }) == false
        )
    {
        std::this_thread::yield();
    }
    work_thread.join();
    while (CurrentThreadTaskQueueRef().ProcessTask() == 0)
    {
        std::this_thread::yield();
    }
}

