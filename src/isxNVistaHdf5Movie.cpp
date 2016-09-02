#include "isxNVistaHdf5Movie.h"
#include "isxHdf5Utils.h"
#include "isxMutex.h"
#include "isxIoQueue.h"
#include "isxConditionVariable.h"

#include <iostream>
#include <vector>
#include <sstream>
#include <mutex>
#include <memory>
#include <cmath>

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
    const SpacingInfo & inSpacingInfo)
{
    std::vector<SpH5File_t> files(1, inHdf5FileHandle->get());

    initialize(inFileName, files, inTimingInfo, inSpacingInfo);
}

NVistaHdf5Movie::NVistaHdf5Movie(
    const std::string &inFileName,
    const std::vector<SpHdf5FileHandle_t> & inHdf5FileHandles,
    const TimingInfo & inTimingInfo,
    const SpacingInfo & inSpacingInfo)
{
    std::vector<SpH5File_t> files;
    for (isize_t i(0); i < inHdf5FileHandles.size(); ++i)
    {
        files.push_back(inHdf5FileHandles[i]->get());
    }

    initialize(inFileName, files, inTimingInfo, inSpacingInfo);
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
    SpVideoFrame_t outFrame;
    getFrameAsync(inFrameNumber,
        [&outFrame, &cv, &mutex](const SpVideoFrame_t & inFrame)
        {
            mutex.lock("getFrame async");
            outFrame = inFrame;
            mutex.unlock();
            cv.notifyOne();
        }
    );
    cv.wait(mutex);
    mutex.unlock();
    return outFrame;
}

void
NVistaHdf5Movie::getFrameAsync(isize_t inFrameNumber, MovieGetFrameCB_t inCallback)
{
    // Only get a weak pointer to this, so that we don't bother reading
    // if this has been deleted when the read gets executed.
    std::weak_ptr<NVistaHdf5Movie> weakThis = shared_from_this();

    uint64_t readRequestId = 0;
    {
        ScopedMutex locker(m_pendingReadsMutex, "getFrameAsync");
        readRequestId = m_readRequestCount++;
    }

    auto readIoTask = std::make_shared<IoTask>(
        [weakThis, this, inFrameNumber, inCallback]()
        {
            auto sharedThis = weakThis.lock();
            if (sharedThis)
            {
                inCallback(getFrameInternal(inFrameNumber));
            }
        },
        [weakThis, this, readRequestId, inCallback](AsyncTaskStatus inStatus)
        {
            auto sharedThis = weakThis.lock();
            if (!sharedThis)
            {
                return;
            }

            auto rt = unregisterReadRequest(readRequestId);

            checkAsyncTaskStatus(rt, inStatus, "MosaicMovie::getFrameAsync");
            if (inStatus == AsyncTaskStatus::ERROR_EXCEPTION)
            {
                inCallback(SpVideoFrame_t());
            }
        }
    );

    {
        ScopedMutex locker(m_pendingReadsMutex, "getFrameAsync");
        m_pendingReads[readRequestId] = readIoTask;
    }
    readIoTask->schedule();
}

SpAsyncTaskHandle_t
NVistaHdf5Movie::unregisterReadRequest(uint64_t inReadRequestId)
{
    ScopedMutex locker(m_pendingReadsMutex, "unregisterPendingRead");
    auto ret = m_pendingReads[inReadRequestId];
    m_pendingReads.erase(inReadRequestId);
    return ret;
}

void
NVistaHdf5Movie::cancelPendingReads()
{
    ScopedMutex locker(m_pendingReadsMutex, "cancelPendingReads");
    for (auto & pr: m_pendingReads)
    {
        pr.second->cancel();
    }
    m_pendingReads.clear();
}
    
const isx::TimingInfo &
NVistaHdf5Movie::getTimingInfo() const
{
    return m_timingInfo;
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
    const SpacingInfo & inSpacingInfo)
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
        m_timingInfo = inTimingInfo;
    }
    else
    {
        initTimingInfo(inHdf5Files);
    }

    if (inSpacingInfo.isValid())
    {
        m_spacingInfo = inSpacingInfo;
    }
    else
    {
        initSpacingInfo(inHdf5Files);
    }

    m_valid = true;
    
}

void
NVistaHdf5Movie::initTimingInfo(const std::vector<SpH5File_t> & inHdf5Files)
{
    if (readTimingInfo(inHdf5Files) == false)
    {
        // If we cannot read the timing info, use default values
        m_timingInfo = TimingInfo::getDefault(m_cumulativeFrames[m_cumulativeFrames.size() - 1]);         
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
    SpacingInfo si = getSpacingInfo();
    auto outFrame = std::make_shared<VideoFrame>(
        si,
        sizeof(uint16_t) * si.getNumColumns(),
        1, // numChannels
        getDataType(),
        m_timingInfo.convertIndexToStartTime(inFrameNumber),
        inFrameNumber);

    isize_t newFrameNumber = inFrameNumber;
    isize_t idx = getMovieIndex(inFrameNumber);
    if (idx > 0)
    {
        newFrameNumber = inFrameNumber - m_cumulativeFrames[idx - 1];
    }

    m_movies[idx]->getFrame(newFrameNumber, outFrame);

    return outFrame;
}

bool
NVistaHdf5Movie::readTimingInfo(std::vector<SpH5File_t> inHdf5Files)
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
        totalDurationInSecs *= 1000.0 / double(totalNumFrames);

        isx::DurationInSeconds step = isx::DurationInSeconds(isize_t(std::round(totalDurationInSecs)), 1000);
        isx::Time start = isx::Time(startTime);
        m_timingInfo = isx::TimingInfo(start, step, totalNumFrames);
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
