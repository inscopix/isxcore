#include "isxNVistaHdf5Movie.h"
#include "isxHdf5Utils.h"
#include "isxMutex.h"
#include "isxIoQueue.h"
#include "isxConditionVariable.h"
#include "isxIoTaskTracker.h"

#include <iostream>
#include <vector>
#include <sstream>
#include <mutex>
#include <memory>
#include <cmath>
#include <cstring>

namespace isx
{

NVistaHdf5Movie::NVistaHdf5Movie()
    : m_valid(false)
{
}

NVistaHdf5Movie::NVistaHdf5Movie(
    const std::string &inFileName,
    const SpHdf5FileHandle_t & inHdf5FileHandle,
    const TimingInfo & inTimingInfo,
    const SpacingInfo & inSpacingInfo,
    const std::vector<isize_t> & inDroppedFrames,
    const std::map<std::string, Variant> & inAdditionalProperties)
    : m_ioTaskTracker(new IoTaskTracker<VideoFrame>())
{
    std::vector<SpH5File_t> files(1, inHdf5FileHandle->get());

    initialize(inFileName, files, inTimingInfo, inSpacingInfo, inDroppedFrames, inAdditionalProperties);
}

NVistaHdf5Movie::NVistaHdf5Movie(
    const std::string &inFileName,
    const std::vector<SpHdf5FileHandle_t> & inHdf5FileHandles,
    const TimingInfo & inTimingInfo,
    const SpacingInfo & inSpacingInfo, 
    const std::vector<isize_t> & inDroppedFrames,
    const std::map<std::string, Variant> & inAdditionalProperties)
    : m_ioTaskTracker(new IoTaskTracker<VideoFrame>())
{
    std::vector<SpH5File_t> files;
    for (isize_t i(0); i < inHdf5FileHandles.size(); ++i)
    {
        files.push_back(inHdf5FileHandles[i]->get());
    }

    initialize(inFileName, files, inTimingInfo, inSpacingInfo, inDroppedFrames, inAdditionalProperties);
}

const std::map<std::string, Variant> & 
NVistaHdf5Movie::getAdditionalProperties() const
{
    return m_additionalProperties;
}

bool
NVistaHdf5Movie::isValid() const
{
    return m_valid;
}

SpVideoFrame_t
NVistaHdf5Movie::getFrame(isize_t inFrameNumber)
{
    Mutex mutex;
    ConditionVariable cv;
    mutex.lock("getFrame");
    AsyncTaskResult<SpVideoFrame_t> asyncTaskResult;
    getFrameAsync(inFrameNumber,
        [&asyncTaskResult, &cv, &mutex](AsyncTaskResult<SpVideoFrame_t> inAsyncTaskResult)
        {
            mutex.lock("getFrame async");
            asyncTaskResult = inAsyncTaskResult;
            mutex.unlock();
            cv.notifyOne();
        }
    );
    cv.wait(mutex);
    mutex.unlock();

    return asyncTaskResult.get();   //throws if asyncTaskResult contains an exception
}

void
NVistaHdf5Movie::getFrameAsync(isize_t inFrameNumber, MovieGetFrameCB_t inCallback)
{
    std::weak_ptr<NVistaHdf5Movie> weakThis = shared_from_this();
    GetFrameCB_t getFrameCB = [weakThis, this, inFrameNumber]()
        {
            auto sharedThis = weakThis.lock();
            if (sharedThis)
            {
                return getFrameInternal(inFrameNumber);
            }
            return SpVideoFrame_t();
        };

    m_ioTaskTracker->schedule(getFrameCB, inCallback);
}

void
NVistaHdf5Movie::cancelPendingReads()
{
    m_ioTaskTracker->cancelPendingTasks();
}
    
const isx::TimingInfo &
NVistaHdf5Movie::getTimingInfo() const
{
    return m_timingInfos[0];
}

const isx::TimingInfos_t &
NVistaHdf5Movie::getTimingInfosForSeries() const
{
    return m_timingInfos;
}

const isx::SpacingInfo &
NVistaHdf5Movie::getSpacingInfo() const
{
    return m_spacingInfo;
}

DataType
NVistaHdf5Movie::getDataType() const
{
    return DataType::U16;
}

std::string
NVistaHdf5Movie::getFileName() const
{
    return m_fileName;
}

void
NVistaHdf5Movie::serialize(std::ostream& strm) const
{
    for (isize_t m(0); m < m_movies.size(); ++m)
    {
        if(m > 0)
        {
            strm << "\n";
        }
        strm << m_movies[m]->getPath();
    }
}

void
NVistaHdf5Movie::initialize(
    const std::string & inFileName,
    const std::vector<SpH5File_t> & inHdf5Files,
    const TimingInfo & inTimingInfo,
    const SpacingInfo & inSpacingInfo,
    const std::vector<isize_t> & inDroppedFrames,
    const std::map<std::string, Variant> & inAdditionalProperties)
{
    ISX_ASSERT(inHdf5Files.size());

    m_fileName = inFileName;
    isize_t numFramesAccum = 0;

    for (isize_t f(0); f < inHdf5Files.size(); ++f)
    {
        std::unique_ptr<Hdf5Movie> p( new Hdf5Movie(inHdf5Files[f], "/images") );
        m_movies.push_back(std::move(p));

        if (f > 1)
        {
            if (m_movies[0]->getFrameWidth() != m_movies[f]->getFrameWidth()
                || m_movies[0]->getFrameHeight() != m_movies[f]->getFrameHeight())
            {
                ISX_THROW(isx::ExceptionUserInput, "All input files must have the same dimensions.");
            }
        }

        numFramesAccum += m_movies[f]->getNumFrames();
        m_cumulativeFrames.push_back(numFramesAccum);
    }

    if (inTimingInfo.isValid())
    {
        m_timingInfos = TimingInfos_t{inTimingInfo};
    }
    else
    {
        initTimingInfo(inHdf5Files, inDroppedFrames);
    }

    if (inSpacingInfo.isValid())
    {
        // TODO sweet : Even if the spacing info is valid, we don't trust
        // the width/height because the acquisition software does not store
        // the actual width/height after downsampling.
        // Therefore, we try to get it from the HDF5 file and use the given
        // width/height as a fallback only.
        if (m_movies.size() > 0)
        {
            const SizeInPixels_t numPixels(m_movies[0]->getFrameWidth(), m_movies[0]->getFrameHeight());
            m_spacingInfo = SpacingInfo(numPixels, inSpacingInfo.getPixelSize(), inSpacingInfo.getTopLeft());
        }
        else
        {
            m_spacingInfo = inSpacingInfo;
        }
    }
    else
    {
        initSpacingInfo(inHdf5Files);
    }

    m_additionalProperties = inAdditionalProperties;

    m_valid = true;
    
}

void
NVistaHdf5Movie::initTimingInfo(const std::vector<SpH5File_t> & inHdf5Files, const std::vector<isize_t> & inDroppedFrames)
{
    if (readTimingInfo(inHdf5Files, inDroppedFrames) == false)
    {
        // If we cannot read the timing info, use default values
        isize_t numFrames = m_cumulativeFrames[m_cumulativeFrames.size() - 1] + inDroppedFrames.size();
        TimingInfo defaultTi = TimingInfo::getDefault(numFrames, inDroppedFrames);
        m_timingInfos = TimingInfos_t{defaultTi};
    }    
}

void
NVistaHdf5Movie::initSpacingInfo(const std::vector<SpH5File_t> & inHdf5Files)
{
    if (readSpacingInfo(inHdf5Files) == false)
    {
        m_spacingInfo = SpacingInfo::getDefault(SizeInPixels_t(m_movies[0]->getFrameWidth(), m_movies[0]->getFrameHeight()));
    }
}

SpVideoFrame_t
NVistaHdf5Movie::getFrameInternal(isize_t inFrameNumber)
{
    TimingInfo & ti = m_timingInfos[0];
    SpacingInfo si = getSpacingInfo();
    auto outFrame = std::make_shared<VideoFrame>(
        si,
        sizeof(uint16_t) * si.getNumColumns(),
        1, // numChannels
        getDataType(),
        getTimingInfo().convertIndexToStartTime(inFrameNumber),
        inFrameNumber);
    
    if(ti.isDropped(inFrameNumber))
    {
        std::memset(outFrame->getPixels(), 0, outFrame->getImageSizeInBytes());
        outFrame->setFrameType(VideoFrame::Type::DROPPED);
        return outFrame;
    }

    // The frame was not dropped, shift frame numbers and proceed to read
    isize_t newFrameNumber = ti.timeIdxToRecordedIdx(inFrameNumber);
    isize_t idx = getMovieIndex(newFrameNumber);
    if (idx > 0)
    {
        newFrameNumber = newFrameNumber - m_cumulativeFrames[idx - 1];
    }

    m_movies[idx]->getFrame(newFrameNumber, outFrame);

    return outFrame;
}

bool
NVistaHdf5Movie::readTimingInfo(std::vector<SpH5File_t> inHdf5Files, const std::vector<isize_t> & inDroppedFrames)
{
    H5::DataSet timingInfoDataSet;
    hsize_t totalNumFrames = 0;
    int64_t startTime = 0;
    double totalDurationInSecs = 0;
    bool bInitializedFromFile = true;

    for (isize_t f(0); f < inHdf5Files.size(); ++f)
    {
        if (isx::internal::hasDatasetAtPath(inHdf5Files[f], "/", "timeStamp"))
        {
            std::vector<hsize_t> timingInfoDims;
            std::vector<hsize_t> timingInfoMaxDims;
            hsize_t numFrames = 0;
            std::vector<double> buffer;

            try
            {
                timingInfoDataSet = inHdf5Files[f]->openDataSet("/timeStamp");
                isx::internal::getHdf5SpaceDims(timingInfoDataSet.getSpace(), timingInfoDims, timingInfoMaxDims);

                numFrames = timingInfoDims[0];
                buffer.resize(numFrames);

                timingInfoDataSet.read(buffer.data(), timingInfoDataSet.getDataType());
            }
            catch (const H5::FileIException& error)
            {
                ISX_THROW(isx::ExceptionFileIO,
                    "Failure caused by H5File operations.\n", error.getDetailMsg());
            }

            catch (const H5::DataSetIException& error)
            {
                ISX_THROW(isx::ExceptionDataIO,
                    "Failure caused by DataSet operations.\n", error.getDetailMsg());
            }

            catch (...)
            {
                ISX_ASSERT(false, "Unhandled exception.");
            }

            // Based on MOS-1319, it seems like the length of the timeStamp DataSet
            // can be 0, which means we should give up on trying to get the timing
            // info like this.
            if (numFrames == 0)
            {
                return false;
            }

            // get start time
            if (f == 0)
            {
                startTime = int64_t(buffer[0]);
            }

            totalDurationInSecs += buffer[numFrames - 1] - buffer[0];
            totalNumFrames += numFrames;
        }
        else
        {
            bInitializedFromFile = false;
            break;
        }
    }

    if (bInitializedFromFile)
    {
        totalNumFrames += inDroppedFrames.size();               /// Account for lost frames

        isx::DurationInSeconds step = isx::TimingInfo::s_defaultStep;
        if (totalDurationInSecs > 0)
        {
            step = isx::Ratio::fromDouble(totalDurationInSecs / double(totalNumFrames));
        }

        isx::Time start = isx::Time(startTime);
        m_timingInfos = TimingInfos_t{isx::TimingInfo(start, step, totalNumFrames, inDroppedFrames)};
    }

    return bInitializedFromFile;
}

bool
NVistaHdf5Movie::readSpacingInfo(std::vector<SpH5File_t> inHdf5Files)
{
    bool bInitializedFromFile = true;

    if (isx::internal::hasDatasetAtPath(inHdf5Files[0], "/", "images"))
    {
        try
        {
            H5::DataSet spacingInfoDataSet = inHdf5Files[0]->openDataSet("/images");

            std::vector<hsize_t> spacingInfoDims;
            std::vector<hsize_t> spacingInfoMaxDims;
            isx::internal::getHdf5SpaceDims(spacingInfoDataSet.getSpace(), spacingInfoDims, spacingInfoMaxDims);

            hsize_t height = spacingInfoDims[1];
            hsize_t width = spacingInfoDims[2];

            SizeInPixels_t numPixels(width, height);
            SizeInMicrons_t pixelSize(isx::DEFAULT_PIXEL_SIZE, isx::DEFAULT_PIXEL_SIZE);
            PointInMicrons_t topLeft(0, 0);
            m_spacingInfo = SpacingInfo(numPixels, pixelSize, topLeft);
        }
        catch (const H5::FileIException& error)
        {
            ISX_THROW(isx::ExceptionFileIO,
                    "Failure caused by H5File operations.\n", error.getDetailMsg());
        }
        catch (const H5::DataSetIException& error)
        {
            ISX_THROW(isx::ExceptionDataIO,
                    "Failure caused by DataSet operations.\n", error.getDetailMsg());
        }
        catch (...)
        {
            ISX_ASSERT(false, "Unhandled exception.");
        }          
    }
    else
    {
        bInitializedFromFile = false;
    }

    return bInitializedFromFile;
}

isize_t
NVistaHdf5Movie::getMovieIndex(isize_t inFrameNumber)
{
    isize_t idx = 0;
    while ((inFrameNumber >= m_cumulativeFrames[idx]) && (idx < m_movies.size() - 1))
    {
        ++idx;
    }
    return idx;
}


} // namespace isx
