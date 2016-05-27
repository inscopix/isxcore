#include "isxCore.h"
#include "isxDispatchQueue.h"
#include "catch.hpp"
#include "isxLog.h"

#include <math.h>
#include <array>
#include <vector>
#include <string>
#include <sstream>
#include <chrono>
#include <thread>

TEST_CASE("DispatchQueue", "[core]") {

    isx::CoreInitialize();

    SECTION("initialize") {
        REQUIRE(isx::CoreIsInitialized());
        isx::CoreShutdown();
        REQUIRE(!isx::CoreIsInitialized());
        isx::CoreInitialize();
    }

    SECTION("default queues") {
        REQUIRE(isx::DispatchQueue::poolQueue());
        REQUIRE(isx::DispatchQueue::mainQueue());
    }

    SECTION("run task") {
        bool taskRan = false;
        isx::DispatchQueue::poolQueue()->dispatch([&]()
        {
            taskRan = true;
        });
        for (int i = 0; i < 250; ++i)
        {
            if (taskRan)
            {
                break;
            }
            std::chrono::milliseconds d(2);
            std::this_thread::sleep_for(d);
        }
        REQUIRE(taskRan);
    }

    SECTION("run task with context") {
        int secret = 123;
        int revealed = -1;
        isx::DispatchQueue::tContextTask t = [&](void * inP)
        {
            int * p = (int *) inP;
            *p = secret;
        };
        isx::DispatchQueue::poolQueue()->dispatch(&revealed, t);
        for (int i = 0; i < 250; ++i)
        {
            if (secret == revealed)
            {
                break;
            }
            std::chrono::milliseconds d(2);
            std::this_thread::sleep_for(d);
        }
        REQUIRE(secret == revealed);
    }
    
    SECTION("run task on new worker thread") {
        isx::tDispatchQueue_SP worker = isx::DispatchQueue::create();
        REQUIRE(worker);
        bool taskRan = false;
        worker->dispatch([&]()
        {
            taskRan = true;
        });
        for (int i = 0; i < 250; ++i)
        {
            if (taskRan)
            {
                break;
            }
            std::chrono::milliseconds d(2);
            std::this_thread::sleep_for(d);
        }
        worker->destroy();
        worker.reset();
        REQUIRE(taskRan);
    }

    SECTION("run task with context on new worker thread") {
        int secret = 123;
        int revealed = -1;
        isx::DispatchQueue::tContextTask t = [&](void * inP)
        {
            int * p = (int *) inP;
            *p = secret;
        };
        isx::tDispatchQueue_SP worker = isx::DispatchQueue::create();
        REQUIRE(worker);
        bool taskRan = false;
        worker->dispatch(&revealed, t);
        for (int i = 0; i < 250; ++i)
        {
            if (secret == revealed)
            {
                break;
            }
            std::chrono::milliseconds d(2);
            std::this_thread::sleep_for(d);
        }
        worker->destroy();
        worker.reset();
        REQUIRE(secret == revealed);
    }

    SECTION("run tasks in the pool with mutex locking")
    {
        isx::tDispatchQueue_SP poolQueue = isx::DispatchQueue::poolQueue();
        REQUIRE(poolQueue);

        int n = 100;
        int count = 0;
        std::mutex countMutex;
        isx::DispatchQueue::tTask incTask([&]()
        {
            std::lock_guard<std::mutex> lock(countMutex);
            ++count;
        });

        for (int i = 0; i < n; ++i)
        {
            poolQueue->dispatch(incTask);
        }

        std::chrono::milliseconds duration(200);
        std::this_thread::sleep_for(duration);

        REQUIRE(count == n);
    }

// this test does not work because we need a Qt event loop handler running in the main
// thread.  This test application is not start the event loop on the main thread.
#if 0
    SECTION("run task on main thread") {
        bool taskRan = false;
        bool workerDone = false;
        // test from worker thread
        isx::DispatchQueue::poolQueue()->dispatch([&]()
        {
            // dispatch from worker thread to main thread
            isx::DispatchQueue::mainQueue()->dispatch([&]()
            {
                taskRan = true;
            });
            for (int i = 0; i < 250; ++i)
            {
                if (taskRan)
                {
                    break;
                }
                std::chrono::milliseconds d(2);
                std::this_thread::sleep_for(d);
            }
            workerDone = true;
        });
        for (int i = 0; i < 500; ++i)
        {
            if (workerDone)
            {
                break;
            }
            std::chrono::milliseconds d(2);
            std::this_thread::sleep_for(d);
        }
        REQUIRE(taskRan);
    }
#endif

    isx::CoreShutdown();

}
