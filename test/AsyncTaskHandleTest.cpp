#include "isxCore.h"
#include "isxAsync.h"
#include "isxAsyncTask.h"
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
#include <exception>

TEST_CASE("AsyncTaskHandle", "[core]") {

    isx::CoreInitialize();

    SECTION("exception on worker") {
        bool taskRanUpToException = false;
        isx::AsyncTaskStatus status = isx::AsyncTaskStatus::COMPLETE;
        isx::AsyncProgressCB_t progressCB;
        isx::AsyncFinishedCB_t finishedCB;

        isx::SpAsyncTaskHandle_t asyncTask = std::make_shared<isx::AsyncTask>(
            [&](isx::AsyncCheckInCB_t){
                taskRanUpToException = true;
                ISX_THROW(isx::Exception, "Expected: exception on worker.");
                return isx::AsyncTaskStatus::COMPLETE;
            }, progressCB, finishedCB, isx::AsyncTaskThreadForFinishedCB::USE_MAIN);

        asyncTask->schedule();
        
        for (int i = 0; i < 250; ++i)
        {
            if (taskRanUpToException
                && asyncTask->getTaskStatus() == isx::AsyncTaskStatus::ERROR_EXCEPTION)
            {
                break;
            }
            std::chrono::milliseconds d(2);
            std::this_thread::sleep_for(d);
        }

        REQUIRE(taskRanUpToException);
        REQUIRE(asyncTask->getTaskStatus() == isx::AsyncTaskStatus::ERROR_EXCEPTION);
        try
        {
            std::rethrow_exception(asyncTask->getExceptionPtr());
        }
        catch(isx::Exception & e)
        {
            REQUIRE(std::string(e.what()) == "Expected: exception on worker.");
        }
    }

    isx::CoreShutdown();
}
