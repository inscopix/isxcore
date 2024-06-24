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
        isx::AsyncTaskStatus status = isx::AsyncTaskStatus::PENDING;
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

    SECTION("IoTask cancellation") {
        bool slowTaskRan = false;
        bool ioTaskRan = false;
        isx::AsyncTaskStatus slowTaskStatus = isx::AsyncTaskStatus::PENDING;
        isx::AsyncTaskStatus ioTaskStatus = isx::AsyncTaskStatus::PENDING;
        
        auto slowTask = std::make_shared<isx::IoTask>(
            [&]()
            {
                std::chrono::milliseconds d(2);
                std::this_thread::sleep_for(d);
                slowTaskRan = true;
            },
            [&](isx::AsyncTaskStatus inStatus){
                slowTaskStatus = inStatus;
            });
        auto ioTask = std::make_shared<isx::IoTask>(
            [&]()
            {
                ioTaskRan = true;
            },
            [&](isx::AsyncTaskStatus inStatus){
                ioTaskStatus = inStatus;
            });
        slowTask->schedule();
        ioTask->schedule();
        ioTask->cancel();
        for (int i = 0; i < 250; ++i)
        {
            if (slowTaskRan)
            {
                break;
            }
            std::chrono::milliseconds d(2);
            std::this_thread::sleep_for(d);
        }
        REQUIRE(slowTaskRan);
        REQUIRE(slowTaskStatus == isx::AsyncTaskStatus::COMPLETE);
        REQUIRE(!ioTaskRan);
        REQUIRE(ioTaskStatus == isx::AsyncTaskStatus::CANCELLED);
    }
    
    isx::CoreShutdown();
}
