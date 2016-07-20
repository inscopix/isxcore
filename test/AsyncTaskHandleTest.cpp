#include "isxCore.h"
#include "isxCoreFwd.h"
#include "isxAsyncTaskHandle.h"
#include "isxConditionVariable.h"
#include "isxException.h"
#include "isxMutex.h"
#include "catch.hpp"
#include "isxLog.h"

#include <sstream>
#include <chrono>
#include <thread>
#include <atomic>

TEST_CASE("AsyncTaskHandle", "[core]") {

    isx::CoreInitialize();

    SECTION("exception on worker") {
        bool taskRanUpToException = false;
        isx::AsyncTaskFinishedStatus status = isx::AsyncTaskFinishedStatus::COMPLETE;
        isx::AsyncTaskHandle::ProgressCB_t progressCB;
        isx::AsyncTaskHandle::FinishedCB_t finishedCB;

        isx::SpAsyncTaskHandle_t asyncTask = std::make_shared<isx::AsyncTaskHandle>(
            [&](isx::AsyncTaskHandle::CheckInCB_t){
                taskRanUpToException = true;
                ISX_THROW(isx::Exception, "Expected exception on worker.\n");
                return isx::AsyncTaskFinishedStatus::COMPLETE;
            }, progressCB, finishedCB);

        asyncTask->process();
        
        for (int i = 0; i < 250; ++i)
        {
            if (taskRanUpToException
                && asyncTask->getTaskStatus() == isx::AsyncTaskFinishedStatus::ERROR_EXCEPTION)
            {
                break;
            }
            std::chrono::milliseconds d(2);
            std::this_thread::sleep_for(d);
        }

        REQUIRE(taskRanUpToException);
        REQUIRE(asyncTask->getTaskStatus() == isx::AsyncTaskFinishedStatus::ERROR_EXCEPTION);
    }

    isx::CoreShutdown();
}
