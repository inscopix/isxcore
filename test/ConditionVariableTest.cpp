#include "isxCore.h"
#include "isxDispatchQueue.h"
#include "isxDispatchQueueWorker.h"
#include "isxConditionVariable.h"
#include "isxMutex.h"
#include "catch.hpp"
#include "isxLog.h"

#include <sstream>
#include <chrono>
#include <thread>
#include <atomic>

TEST_CASE("ConditionVariable", "[core]") {

    isx::CoreInitialize();

    SECTION("sync worker thread with cond variable") {
        isx::SpDispatchQueueWorker_t worker(new isx::DispatchQueueWorker());
        REQUIRE(worker);
        bool taskRan = false;
        isx::Mutex mutex;
        isx::ConditionVariable cv;
        mutex.lock("main");
        worker->dispatch([&]()
        {
            mutex.lock("worker");
            taskRan = true;
            mutex.unlock();
            cv.notifyOne();
        });
        bool didNotTimeout = cv.waitForMs(mutex, 50);
        mutex.unlock();

        worker->destroy();
        worker.reset();
        REQUIRE(didNotTimeout);
        REQUIRE(taskRan);
    }

    isx::CoreShutdown();
}
