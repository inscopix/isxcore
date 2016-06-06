#include "isxCore.h"
#include "isxDispatchQueue.h"
#include "isxDispatchQueueWorker.h"
#include "isxMutex.h"
#include "catch.hpp"
#include "isxLog.h"

#include <sstream>
#include <chrono>
#include <thread>
#include <atomic>

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
        isx::ContextTask_t t = [&](void * inP)
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
        isx::SpDispatchQueueWorker_t worker(new isx::DispatchQueueWorker());
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
        isx::ContextTask_t t = [&](void * inP)
        {
            int * p = (int *) inP;
            *p = secret;
        };
        isx::SpDispatchQueueWorker_t worker(new isx::DispatchQueueWorker());
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

    SECTION("mutex locking to avoid race between two workers")
    {
        auto w0 = std::make_shared<isx::DispatchQueueWorker>();
        auto w1 = std::make_shared<isx::DispatchQueueWorker>();

        int32_t n = 500;
        int32_t count = 0;
        bool doSleep = true;
        isx::Mutex countMutex;
        isx::ContextTask_t incTask([&](void * inSleep)
        {
            std::string name = "task0";
            if (inSleep && *(bool *)inSleep)
            {
                name[4] = '1';
            }

            isx::ScopedMutex guard(countMutex, name);

            // read
            int32_t readCount = count;
            
            // sleep
            if (inSleep && *(bool *)inSleep)
            {
                std::chrono::microseconds d(1);
                std::this_thread::sleep_for(d);
            }
            
            // write
            count = readCount + 1;
        });

        std::atomic_int doneCount(0);
        isx::Task_t lastTask([&]()
        {
            doneCount++;
        });

        for (int i = 0; i < n; ++i)
        {
            w0->dispatch(&doSleep, incTask);
            w1->dispatch(0, incTask);
        }
        
        w0->dispatch(lastTask);
        w1->dispatch(lastTask);

        for (int i = 0; i < 250000; ++i)
        {
            if (doneCount == 2)
            {
                break;
            }
            std::chrono::microseconds d(1);
            std::this_thread::sleep_for(d);
        }

        w0->destroy();
        w1->destroy();
        
        REQUIRE(count == 2 * n);
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
