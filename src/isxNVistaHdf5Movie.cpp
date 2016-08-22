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
    const SpHdf5FileHandle_t & inHdf5FileHandle,
    const TimingInfo & inTimingInfo,
    const SpacingInfo & inSpacingInfo)
{
    std::vector<SpH5File_t> files(1, inHdf5FileHandle->get());

    initialize(files, inTimingInfo, inSpacingInfo);
}

NVistaHdf5Movie::NVistaHdf5Movie(
    const std::vector<SpHdf5FileHandle_t> & inHdf5FileHandles,
    const TimingInfo & inTimingInfo,
    const SpacingInfo & inSpacingInfo)
{
    std::vector<SpH5File_t> files;
    for (isize_t i(0); i < inHdf5FileHandles.size(); ++i)
    {
        files.push_back(inHdf5FileHandles[i]->get());
    }

    initialize(files, inTimingInfo, inSpacingInfo);
}

NVistaHdf5Movie::~NVistaHdf5Movie()
{
}

bool
NVistaHdf5Movie::isValid() const
{
    return m_valid;
}

void
NVistaHdf5Movie::getFrame(isize_t inFrameNumber, SpU16VideoFrame_t & outFrame)
{
    getFrameTemplate<U16VideoFrame_t>(inFrameNumber, outFrame);
}

void
NVistaHdf5Movie::getFrame(isize_t inFrameNumber, SpF32VideoFrame_t & outFrame)
{
    getFrameTemplate<F32VideoFrame_t>(inFrameNumber, outFrame);
}

void
NVistaHdf5Movie::getFrameAsync(isize_t inFrameNumber, MovieGetU16FrameCB_t inCallback)
{
    getFrameAsyncTemplate<U16VideoFrame_t>(inFrameNumber, inCallback);
}

void
NVistaHdf5Movie::getFrameAsync(isize_t inFrameNumber, MovieGetF32FrameCB_t inCallback)
{
    getFrameAsyncTemplate<F32VideoFrame_t>(inFrameNumber, inCallback);
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
NVistaHdf5Movie::getName() const
{
    std::string path = m_movies[0]->getPath();
    std::string name = path.substr(path.find_last_of("/") + 1);
    return name;
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
    const std::vector<SpH5File_t> & inHdf5Files,
    const TimingInfo & inTimingInfo,
    const SpacingInfo & inSpacingInfo)
{
    ISX_ASSERT(inHdf5Files.size());

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

void
NVistaHdf5Movie::getFrameInternal(isize_t inFrameNumber, SpU16VideoFrame_t & outFrame)
{
    Time frameTime = m_timingInfo.convertIndexToTime(inFrameNumber);
    SpacingInfo si = getSpacingInfo();
    outFrame = std::make_shared<U16VideoFrame_t>(
        si,
        sizeof(uint16_t) * si.getNumColumns(),
        1, // numChannels
        frameTime,
        inFrameNumber);

    isize_t newFrameNumber = inFrameNumber;
    isize_t idx = getMovieIndex(inFrameNumber);
    if (idx > 0)
    {
        newFrameNumber = inFrameNumber - m_cumulativeFrames[idx - 1];
    }

    m_movies[idx]->getFrame(newFrameNumber, outFrame);
}

void
NVistaHdf5Movie::getFrameInternal(isize_t inFrameNumber, SpF32VideoFrame_t & outFrame)
{
    // Get the uint16 frame
    SpU16VideoFrame_t u16Frame;
    getFrameInternal(inFrameNumber, u16Frame);

    // then cast it
    SpacingInfo si = u16Frame->getImage().getSpacingInfo();
    Time frameTime = u16Frame->getTimeStamp();
    outFrame = std::make_shared<F32VideoFrame_t>(
        si,
        sizeof(float) * si.getNumColumns(),
        1, // numChannels
        frameTime,
        inFrameNumber);

    isize_t numPixels = si.getTotalNumPixels();
    uint16_t * u16FrameArray = u16Frame->getPixels();
    float * outFrameArray = outFrame->getPixels();
    for (isize_t i = 0; i < numPixels; ++i)
    {
        outFrameArray[i] = float(u16FrameArray[i]);
    }
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

template <typename FrameType>
void
NVistaHdf5Movie::getFrameTemplate(
        isize_t inFrameNumber,
        std::shared_ptr<FrameType> & outFrame)
{
    Mutex mutex;
    ConditionVariable cv;
    mutex.lock("getFrame");
    getFrameAsync(inFrameNumber,
        [&outFrame, &cv, &mutex](const std::shared_ptr<FrameType> & inFrame)
        {
            mutex.lock("getFrame async");
            outFrame = inFrame;
            mutex.unlock();
            cv.notifyOne();
        }
    );
    cv.wait(mutex);
    mutex.unlock();
}

template <typename FrameType>
void
NVistaHdf5Movie::getFrameAsyncTemplate(
        isize_t inFrameNumber,
        MovieGetFrameCB_t<FrameType> inCallback)
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
                std::shared_ptr<FrameType> frame;
                getFrameInternal(inFrameNumber, frame);
                inCallback(frame);
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

            switch (inStatus)
            {
                case AsyncTaskStatus::ERROR_EXCEPTION:
                {
                    try
                    {
                        std::rethrow_exception(rt->getExceptionPtr());
                    }
                    catch(std::exception & e)
                    {
                        ISX_LOG_ERROR("Exception occurred reading from NVistaHdf5Movie: ", e.what());
                    }
                    std::shared_ptr<FrameType> frame;
                    inCallback(frame);
                    break;
                }

                case AsyncTaskStatus::UNKNOWN_ERROR:
                    ISX_LOG_ERROR("An error occurred while reading a frame from a NVistaHdf5Movie file");
                    break;

                case AsyncTaskStatus::CANCELLED:
                    ISX_LOG_INFO("getFrameAsync request cancelled.");
                    break;

                case AsyncTaskStatus::COMPLETE:
                case AsyncTaskStatus::PENDING:      // won't happen - case is here only to quiet compiler
                case AsyncTaskStatus::PROCESSING:   // won't happen - case is here only to quiet compiler
                    break;
            }
        }
    );

    {
        ScopedMutex locker(m_pendingReadsMutex, "getFrameAsync");
        m_pendingReads[readRequestId] = readIoTask;
    }
    readIoTask->schedule();
}

} // namespace isx
