#include "isxMovieFactory.h"
#include "isxMosaicMovie.h"
#include "isxNVistaHdf5Movie.h"
#include "isxBehavMovie.h"
#include "isxMovieSeries.h"
#include "isxRecording.h"
#include "isxPathUtils.h"

#include <ctype.h>
#include <algorithm>

namespace isx
{

SpWritableMovie_t
writeMosaicMovie(
        const std::string & inFileName,
        const TimingInfo & inTimingInfo,
        const SpacingInfo & inSpacingInfo,
        DataType inDataType)
{
    SpWritableMovie_t movie = std::make_shared<MosaicMovie>(
            inFileName, inTimingInfo, inSpacingInfo, inDataType);
    return movie;
}

SpMovie_t
readMovie(const std::string & inFileName)
{
    std::string ext = getExtension(inFileName);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    if (ext == "isxd")
    {
        return readMosaicMovie(inFileName);
    }
    else if (isNVistaImagingFileExtension(inFileName))
    {
        return readInscopixMovie(inFileName);
    }
    else
    {
        ISX_THROW(ExceptionDataIO, "Movie extension not recognized: ", ext);
    }
}
    
SpMovie_t
readMovieSeries(const std::vector<std::string> & inFileNames, const std::vector<DataSet::Properties> & inProperties)
{
    return std::make_shared<MovieSeries>(inFileNames, inProperties);
}

SpMovie_t
readMosaicMovie(const std::string & inFileName)
{
    SpMovie_t movie = std::make_shared<MosaicMovie>(inFileName);
    return movie;
}

SpMovie_t
readInscopixMovie(const std::string & inFileName)
{
    SpRecording_t recording = std::make_shared<Recording>(inFileName);
    SpMovie_t movie = recording->getMovie();
    return movie;
}

SpMovie_t
readBehavioralMovie(const std::string & inFileName, const DataSet::Properties & inProperties)
{
    SpMovie_t movie = std::make_shared<BehavMovie>(inFileName, inProperties);
    return movie;
}

bool
isBehavioralMovieFileExtension(const std::string & inFileName)
{
    auto e = isx::getExtension(inFileName);
    std::transform(e.begin(), e.end(), e.begin(), ::tolower);
    return ((e == "mpg") || (e == "mpeg") || (e == "mp4") || (e == "avi") || (e == "wmv") );
}

bool
isNVistaImagingFileExtension(const std::string & inFileName)
{
    auto e = isx::getExtension(inFileName);
    std::transform(e.begin(), e.end(), e.begin(), ::tolower);
    return (e == "hdf5") || (e == "xml") || (e == "tif");
}

} // namespace isx
