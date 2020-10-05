#include "isxMetadata.h"
#include "isxCellSetFactory.h"
#include "isxPathUtils.h"
#include "isxCore.h"
#include "isxTest.h"

#include "catch.hpp"

TEST_CASE("InjectMetadata", "[core][metadata]")
{
    isx::CoreInitialize();

    SECTION("cell set")
    {
        const std::string inCellSetFilename = g_resources["unitTestDataPath"] + "/dual_color/static_red-PP-ROI.isxd";
        isx::SpCellSet_t cellSet = isx::readCellSet(inCellSetFilename);

        ISX_LOG_INFO("before: ", static_cast<int>(isx::getCellSetMethod(cellSet)));
//        isx::setCellSetMethod(cellSet, isx::CellSetMethod_t::CNMFE);
        ISX_LOG_INFO("after: ", static_cast<int>(isx::getCellSetMethod(cellSet)));

//        REQUIRE(1==0);
    }

    isx::CoreShutdown();
}
