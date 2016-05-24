#include "isxDispatchQueue.h"
#include "catch.hpp"
#include "isxLog.h"

#include <vector>
#include <chrono>
#include <thread>

TEST_CASE("DispatchQueue", "[core]") {

//    SECTION("default queue") {
//        REQUIRE(!!isx::DispatchQueue::defaultQueue());
//    }

    SECTION("run task") {
        bool taskRan = false;
        REQUIRE(!taskRan);
        isx::DispatchQueue::defaultQueue().dispatch([&]()
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
        REQUIRE(secret != revealed);
        isx::DispatchQueue::defaultQueue().dispatch(&revealed, t);
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
    
// this test does not work because we need a Qt event loop handler running in the main
// thread and this test application is not a Qt application (and the main thread is not
// a QThread.
#if 0
    SECTION("run task on main thread") {
        bool taskRan = false;
        bool workerDone = false;
        // test from worker thread
        isx::DispatchQueue::defaultQueue().dispatch([&]()
        {
            // dispatch from worker thread to main thread
            isx::DispatchQueue::mainQueue().dispatch([&]()
            {
                taskRan = true;
            });
            for (int i = 0; i < 2500; ++i)
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
}
