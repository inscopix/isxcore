#include "isxVesselSetFactory.h"
#include "isxVesselSetSimple.h"
#include "isxVesselSetSeries.h"
#include <memory>


namespace isx
{

SpVesselSet_t writeVesselSet(
        const std::string & inFileName,
        const TimingInfo & inTimingInfo,
        const SpacingInfo & inSpacingInfo,
        const VesselSetType_t inVesselSetType)
{
    SpVesselSet_t vs = std::make_shared<VesselSetSimple>(inFileName, inTimingInfo, inSpacingInfo, inVesselSetType);
    return vs;
}

SpVesselSet_t readVesselSet(const std::string & inFileName, bool enableWrite)
{
    SpVesselSet_t vs = std::make_shared<VesselSetSimple>(inFileName, enableWrite);
    return vs;
}

SpVesselSet_t readVesselSetSeries(const std::vector<std::string> & inFileNames, bool enableWrite)
{
    SpVesselSet_t vs = std::make_shared<VesselSetSeries>(inFileNames, enableWrite);
    return vs;
}

}
