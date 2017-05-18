#include "isxCellSetFactory.h"
#include "isxCellSetSimple.h"
#include "isxCellSetSeries.h"
#include <memory>


namespace isx
{

SpCellSet_t writeCellSet(
        const std::string & inFileName,
        const TimingInfo & inTimingInfo,
        const SpacingInfo & inSpacingInfo,
        const bool inIsRoiSet)
{
    SpCellSet_t cs = std::make_shared<CellSetSimple>(inFileName, inTimingInfo, inSpacingInfo, inIsRoiSet);
    return cs;
}


SpCellSet_t readCellSet(const std::string & inFileName, bool enableWrite)
{
    SpCellSet_t cs = std::make_shared<CellSetSimple>(inFileName, enableWrite);
    return cs;
}


SpCellSet_t readCellSetSeries(const std::vector<std::string> & inFileNames, bool enableWrite)
{
    SpCellSet_t cs = std::make_shared<CellSetSeries>(inFileNames, enableWrite);
    return cs;
}

}
