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

    SECTION("run task", "[core]") {
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

    SECTION("run task with context", "[core]") {
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
}
