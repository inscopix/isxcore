#include "isxCore.h"
#include "isxIoQueue.h"
#include "isxMutex.h"
#include "catch.hpp"
#include "isxLog.h"

#include <chrono>
#include <thread>


TEST_CASE("IoQueue", "[core]") {

    isx::CoreInitialize();

    SECTION("queue instance") {
        REQUIRE(isx::IoQueue::instance() != nullptr);
    }

    SECTION("dispatch task to IoQueue") {
        bool taskRan = false;
        isx::AsyncTaskStatus status = isx::AsyncTaskStatus::UNKNOWN_ERROR;
        auto ioTask = std::make_shared<isx::IoTask>(
            [&]()
            {
                taskRan = true;
            },
            [&](isx::AsyncTaskStatus inStatus){
                status = inStatus;
            });
        ioTask->schedule();
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
        REQUIRE(status == isx::AsyncTaskStatus::COMPLETE);
    }

    isx::CoreShutdown();
}
