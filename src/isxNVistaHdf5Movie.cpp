#include "isxNVistaHdf5Movie.h"
#include "isxHdf5Utils.h"
#include "isxMutex.h"
#include "isxConditionVariable.h"
#include "isxIoQueue.h"
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
    const SpHdf5FileHandle_t & inHdf5FileHandle)
{
    std::vector<SpH5File_t> files(1, inHdf5FileHandle->get());
    initialize(files);
}

NVistaHdf5Movie::NVistaHdf5Movie(
    const std::vector<SpHdf5FileHandle_t> & inHdf5FileHandles)
{
    std::vector<SpH5File_t> files;
    for (isize_t i(0); i < inHdf5FileHandles.size(); ++i)
    {
        files.push_back(inHdf5FileHandles[i]->get());
    }
    initialize(files);
}

NVistaHdf5Movie::~NVistaHdf5Movie()
{
}


bool
NVistaHdf5Movie::isValid() const
{
    return m_valid;
}

SpU16VideoFrame_t
NVistaHdf5Movie::getFrame(isize_t inFrameNumber)
{
    SpU16VideoFrame_t ret;
    Mutex mutex;
    ConditionVariable cv;
    mutex.lock("getFrame");
    getFrameAsync(inFrameNumber, [&ret, &cv, &mutex](const SpU16VideoFrame_t & inVideoFrame){
        mutex.lock("getFrame async");
        mutex.unlock();
        ret = inVideoFrame;
        cv.notifyOne();
    });
    bool didNotTimeOut = cv.waitForMs(mutex, 500);
    mutex.unlock();
    if (didNotTimeOut == false)
    {
        ISX_THROW(isx::ExceptionDataIO, "getFrame timed out.\n");
    }
    return ret;
}

SpU16VideoFrame_t
NVistaHdf5Movie::getFrame(const Time & inTime)
{
    isize_t frameNumber = m_timingInfo.convertTimeToIndex(inTime);
    return getFrame(frameNumber);
}

void
NVistaHdf5Movie::getFrameAsync(isize_t inFrameNumber, MovieGetFrameCB_t inCallback)
{
    WpNVistaHdf5Movie_t weakThis = shared_from_this();
    IoQueue::instance()->enqueue(
        IoQueue::IoTask(
            [weakThis, this, inFrameNumber, inCallback]()
            {
                SpNVistaHdf5Movie_t sharedThis = weakThis.lock();
                if (sharedThis)
                {
                    inCallback(getFrameInternal(inFrameNumber));
                }
            },
            [inCallback](AsyncTaskStatus inStatus)
            {
                switch (inStatus)
                {
                    case AsyncTaskStatus::ERROR_EXCEPTION:
                        ISX_LOG_ERROR("An exception occurred while reading a frame from an NVistaHdf5Movie file.");
                        inCallback(SpU16VideoFrame_t());
                        break;

                    case AsyncTaskStatus::UNKNOWN_ERROR:
                        ISX_LOG_ERROR("An error occurred while reading a frame from an NVistaHdf5Movie file");
                        inCallback(SpU16VideoFrame_t());
                        break;

                    case AsyncTaskStatus::CANCELLED:
                        ISX_LOG_ERROR("getFrameAsync request cancelled.");
                        break;

                    case AsyncTaskStatus::COMPLETE:
                    case AsyncTaskStatus::PENDING:      // won't happen - case is here only to quiet compiler
                    case AsyncTaskStatus::PROCESSING:   // won't happen - case is here only to quiet compiler
                        break;
                }
            }
    ));
}

void
NVistaHdf5Movie::getFrameAsync(const Time & inTime, MovieGetFrameCB_t inCallback)
{
    isize_t frameNumber = m_timingInfo.convertTimeToIndex(inTime);
    return getFrameAsync(frameNumber, inCallback);
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
    const std::vector<SpH5File_t> & inHdf5Files)
{
    ISX_ASSERT(inHdf5Files.size());

    isize_t w, h;
    isize_t numFramesAccum = 0;

    for (isize_t f(0); f < inHdf5Files.size(); ++f)
    {
        std::unique_ptr<Hdf5Movie> p( new Hdf5Movie(inHdf5Files[f], "/images") );
        m_movies.push_back(std::move(p));

        if (f > 1)
        {
            ISX_ASSERT(w == m_movies[f]->getFrameWidth());
            ISX_ASSERT(h == m_movies[f]->getFrameHeight());
        }
        else
        {
            w = m_movies[f]->getFrameWidth();
            h = m_movies[f]->getFrameHeight();
        }

        numFramesAccum += m_movies[f]->getNumFrames();
        m_cumulativeFrames.push_back(numFramesAccum);
    }

    // TODO michele 2016/07/08 : time since epoch comes from host machine and frame rate 
    // is calculated, so these values are not what we really want. we should want to 
    // pull these from the xml eventually
    if (readTimingInfo(inHdf5Files) == false)
    {
        // If we cannot read the timing info, use default values
        Time start;                       // Default to Unix epoch
        DurationInSeconds step(50, 1000); // Default to 20Hz
        m_timingInfo = TimingInfo(start, step, numFramesAccum);
        DurationInSeconds gstep = m_timingInfo.getStep();
    }
    m_spacingInfo = createDummySpacingInfo(m_movies[0]->getFrameWidth(), m_movies[0]->getFrameHeight());
    m_valid = true;
}

SpU16VideoFrame_t
NVistaHdf5Movie::getFrameInternal(isize_t inFrameNumber)
{
    Time frameTime = m_timingInfo.convertIndexToTime(inFrameNumber);
    SpacingInfo si = getSpacingInfo();
    auto nvf = std::make_shared<U16VideoFrame_t>(
        si.getNumColumns(), si.getNumRows(),
        sizeof(uint16_t) * si.getNumColumns(),
        1, // numChannels
        frameTime, inFrameNumber);

    isize_t newFrameNumber = inFrameNumber;
    isize_t idx = getMovieIndex(inFrameNumber);
    if (idx > 0)
    {
        newFrameNumber = inFrameNumber - m_cumulativeFrames[idx - 1];
    }

    m_movies[idx]->getFrame(newFrameNumber, nvf);
    return nvf;
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

SpacingInfo
NVistaHdf5Movie::createDummySpacingInfo(isize_t width, isize_t height)
{
    SizeInPixels_t numPixels(width, height);
    SizeInMicrons_t pixelSize(Ratio(22, 10), Ratio(22, 10));
    PointInMicrons_t topLeft(0, 0);
    return SpacingInfo(numPixels, pixelSize, topLeft);
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
