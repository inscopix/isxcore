#include "isxCellSetFactory.h"
#include "isxCellSetSimple.h"
#include "isxCellSetSeries.h"
#include <memory>


namespace isx
{

SpCellSet_t writeCellSet(
        const std::string & inFileName,
        const TimingInfo & inTimingInfo,
        const SpacingInfo & inSpacingInfo)
{
    SpCellSet_t cs = std::make_shared<CellSetSimple>(inFileName, inTimingInfo, inSpacingInfo);
    return cs;
}


SpCellSet_t readCellSet(const std::string & inFileName)
{
    SpCellSet_t cs = std::make_shared<CellSetSimple>(inFileName);
    return cs;
}


SpCellSet_t readCellSetSeries(const std::vector<std::string> & inFileNames)
{
    SpCellSet_t cs = std::make_shared<CellSetSeries>(inFileNames);
    return cs;
}

}