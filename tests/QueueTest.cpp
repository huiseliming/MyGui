#include <atomic>
#include <mutex>
#include <random>
#include <thread>
#include <iostream>
#include <vector>
#include <catch2/catch.hpp>
#include "Queue.h"
#include "ThreadTaskQueue.h"

TEST_CASE("SPSC Queue")
{
    int32_t test_enqueue_number_counter = 9999;
    TQueue<int32_t*, QM_SPSC> spsc_queue;
    std::thread number_enqueue_thread([&]{
        for (int32_t i = 0; i < test_enqueue_number_counter;i++) {
            spsc_queue.Enqueue(new int32_t(i));
        }
    });
    std::thread number_dequeue_thread([&]{
        int32_t counter = 0;
        int32_t* value;
        while (counter != test_enqueue_number_counter) {
            while (spsc_queue.Dequeue(value)) {
                REQUIRE(counter == *value);
                delete value;
                counter++;
            }
        }
    });
    number_enqueue_thread.join();
    number_dequeue_thread.join();
}

TEST_CASE("MPSC Queue")
{
    int32_t producer_thread_number = 99;
    int32_t test_enqueue_number_counter = 9999;
    std::atomic<int64_t> enqueue_number_sum = 0;
    std::atomic<int64_t> dequeue_number_sum = 0;
    TQueue<int32_t*, QM_MPSC> mpsc_queue;
    std::vector<std::thread> number_enqueue_threads;

    for (int32_t i = 0; i < producer_thread_number;i++) {
        number_enqueue_threads.push_back(std::thread([&]{
            std::random_device random_device;
            std::mt19937 random_engine(random_device()); // or std::default_random_engine e{rd()};
            std::uniform_int_distribution<int> uniform_int_distribution(1, 9999);
            int64_t random_number_sum = 0;
            for (int32_t i = 0; i < test_enqueue_number_counter;i++) {
                int32_t random_number = uniform_int_distribution(random_device);
                random_number_sum += random_number;
                mpsc_queue.Enqueue(new int32_t(random_number));
            }
            enqueue_number_sum.fetch_add(random_number_sum);
        }));
    }
    std::thread number_output_thread([&]{
        int32_t counter = 0;
        int32_t* value;
        while (true) {
            while (mpsc_queue.Dequeue(value)) {
                dequeue_number_sum += *value;
                delete value;
                counter++;
            }
            if (counter == producer_thread_number * test_enqueue_number_counter) {
                break;
            }
        }
    });
    for (int32_t i = 0; i < producer_thread_number;i++) {
        number_enqueue_threads[i].join();
    }
    number_output_thread.join();
    REQUIRE(enqueue_number_sum.load() == dequeue_number_sum.load());
}
