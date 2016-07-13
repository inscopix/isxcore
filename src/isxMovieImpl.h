#ifndef ISX_MOVIEIMPL_H
#define ISX_MOVIEIMPL_H
#include "isxCore.h"
#include "isxMovieDefs.h"     // For MovieGetFrameCB_t & SpU16VideoFrame_t
#include "isxIoQueue.h"
#include "isxTimingInfo.h"
#include "isxSpacingInfo.h"
#include <queue>



namespace isx 
{
    /// A base class for the private implementation of movies
    /// 
    class MovieImpl : public std::enable_shared_from_this<MovieImpl>
    {
    public: 
        
        /// API for getting a frame from a movie
        /// \param inFrameNumber the frame index
        /// \return the video frame
        virtual SpU16VideoFrame_t getFrame(isize_t inFrameNumber) = 0;

        /// \return the video frame with a time point as an input
        /// \param inTime desired time point
        SpU16VideoFrame_t
        getFrameByTime(const Time & inTime)
        {
            isize_t frameNumber = m_timingInfo.convertTimeToIndex(inTime);
            return getFrame(frameNumber);
        }

        /// Get a frame asynchronously
        /// \param inFrameNumber    frame number
        /// \param inCallback   callback to execute when finished
        void
        getFrameAsync(isize_t inFrameNumber, MovieGetFrameCB_t inCallback)
        {
            {
                isx::ScopedMutex locker(m_frameRequestQueueMutex, "getFrameAsync");
                m_frameRequestQueue.push(FrameRequest(inFrameNumber, inCallback));
            }
            processFrameQueue();
        }

        /// Get frame asynchronously by time
        /// \param inTime    time point
        /// \param inCallback   callback to execute when finished
        void
        getFrameAsync(const Time & inTime, MovieGetFrameCB_t inCallback)
        {
            isize_t frameNumber = m_timingInfo.convertTimeToIndex(inTime);
            return getFrameAsync(frameNumber, inCallback);
        }

        /// \return the duration of the movie in seconds
        ///
        double
        getDurationInSeconds() const
        {
            return m_timingInfo.getDuration().toDouble();
        }

        /// \return timing information for the movie
        ///
        const isx::TimingInfo &
        getTimingInfo() const
        {
            return m_timingInfo;
        }

        /// \return timing information for the movie
        ///
        const SpacingInfo &
        getSpacingInfo() const
        {
            return m_spacingInfo;
        }

        /// \return the total number of frames
        ///
        isize_t
        getNumFrames() const
        {
            return m_timingInfo.getNumTimes();
        }

        /// \return     The width of a frame in pixels.
        ///
        isize_t
        getFrameWidth() const
        {
            return m_spacingInfo.getNumPixels().getWidth();
        }

        /// \return     The height of a frame in pixels.
        ///
        isize_t
        getFrameHeight() const
        {
            return m_spacingInfo.getNumPixels().getHeight();
        }

        /// \return     The size of a frame in bytes.
        ///
        isize_t
        getFrameSizeInBytes() const
        {
            return m_spacingInfo.getTotalNumPixels() * sizeof(uint16_t);
        }

    protected:
    
        /// a class representing a frame request
        ///
        class FrameRequest
        {
        public:
            /// constructor
            /// \param inFrameNumber    frame number requested
            /// \param inCallback       callback to execute when finished
            FrameRequest(isize_t inFrameNumber, MovieGetFrameCB_t inCallback)
                : m_frameNumber(inFrameNumber)
                , m_callback(inCallback) {}

            isize_t             m_frameNumber;  //!< frame index
            MovieGetFrameCB_t   m_callback;     //!< callback to execute
        };

        /// process next frame request
        ///
        void
        processFrameQueue()
        {
            isx::ScopedMutex locker(m_frameRequestQueueMutex, "processFrameQueue");
            if (!m_frameRequestQueue.empty())
            {
                FrameRequest fr = m_frameRequestQueue.front();
                m_frameRequestQueue.pop();
                WpImpl_t weakThis = shared_from_this();
                IoQueue::instance()->dispatch([weakThis, this, fr]()
                {
                    SpImpl_t sharedThis = weakThis.lock();
                    if (!sharedThis)
                    {
                        fr.m_callback(nullptr);
                    }
                    else
                    {
                        fr.m_callback(getFrame(fr.m_frameNumber));
                        processFrameQueue();
                    }
                });
            }
        }

        /// Purge queue
        ///
        void
        purgeFrameQueue()
        {
            isx::ScopedMutex locker(m_frameRequestQueueMutex, "getFrameAsync");
            while (!m_frameRequestQueue.empty())
            {
                m_frameRequestQueue.pop();
            }
        }

        /// A method to create a dummy TimingInfo object from the number of frames.
        ///
        TimingInfo
        createDummyTimingInfo(isize_t numFrames, Ratio inFrameRate)
        {
            isx::Time start = isx::Time();
            isx::Ratio step = inFrameRate.getInverse();
            return isx::TimingInfo(start, step, numFrames);
        }

        /// A method to create a dummy spacing information from the number of rows and columns.
        ///
        SpacingInfo
        createDummySpacingInfo(isize_t width, isize_t height)
        {
            SizeInPixels_t numPixels(width, height);
            SizeInMicrons_t pixelSize(Ratio(22, 10), Ratio(22, 10));
            PointInMicrons_t topLeft(0, 0);
            return SpacingInfo(numPixels, pixelSize, topLeft);
        }

        isx::TimingInfo m_timingInfo;                   //!< Movie timing information
        isx::SpacingInfo m_spacingInfo;                 //!< Movie spacing information

        std::queue<FrameRequest>    m_frameRequestQueue;        //!< Frame request queue
        isx::Mutex                  m_frameRequestQueueMutex;   //!< Queue mutex

        typedef std::shared_ptr<MovieImpl> SpImpl_t;    //!< shared pointer
        typedef std::weak_ptr<MovieImpl> WpImpl_t;      //!< weak pointer
    };
}

#endif
