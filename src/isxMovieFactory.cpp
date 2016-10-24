#include "isxMovieFactory.h"
#include "isxMosaicMovie.h"
#include "isxNVistaHdf5Movie.h"
#include "isxBehavMovie.h"
#include "isxRecording.h"
#include "isxPathUtils.h"

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
    const std::string extension = getExtension(inFileName);
    if (extension == "isxd")
    {
        return readMosaicMovie(inFileName);
    }
    else if ((extension == "hdf5") || (extension == "xml"))
    {
        return readNVistaHdf5Movie(inFileName);
    }
    else if ((extension == "mpg") || (extension == "mp4"))
    {
        return readBehavioralMovie(inFileName);
    }
    else
    {
        ISX_THROW(isx::ExceptionDataIO,
                "Movie extension not recognized: ", extension);
    }
}

SpMovie_t
readMosaicMovie(const std::string & inFileName)
{
    SpMovie_t movie = std::make_shared<MosaicMovie>(inFileName);
    return movie;
}

SpMovie_t
readNVistaHdf5Movie(const std::string & inFileName)
{
    SpRecording_t recording = std::make_shared<Recording>(inFileName);
    SpMovie_t movie = recording->getMovie();
    return movie;
}
    
SpMovie_t
readBehavioralMovie(const std::string & inFileName)
{
    SpMovie_t movie = std::make_shared<BehavMovie>(inFileName);
    return movie;
}

} // namespace isx
