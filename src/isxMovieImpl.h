#ifndef ISX_MOVIEIMPL_H
#define ISX_MOVIEIMPL_H
#include "isxCore.h"
#include "isxMovieDefs.h"     // For MovieGetFrameCB_t & SpU16VideoFrame_t
#include "isxIoQueue.h"
#include "isxTimingInfo.h"
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
        virtual SpU16VideoFrame_t
        getFrameByTime(const Time & inTime)
        {
            isize_t frameNumber = m_timingInfo.convertTimeToIndex(inTime);
            return getFrame(frameNumber);
        }

        /// Get a frame asynchronously
        /// \param inFrameNumber    frame number
        /// \param inCallback   callback to execute when finished
        virtual void
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
        virtual void
        getFrameAsync(const Time & inTime, MovieGetFrameCB_t inCallback)
        {
            isize_t frameNumber = m_timingInfo.convertTimeToIndex(inTime);
            return getFrameAsync(frameNumber, inCallback);
        }

        /// \return the duration of the movie in seconds
        ///
        virtual double
        getDurationInSeconds() const
        {
            return m_timingInfo.getDuration().toDouble();
        }

        /// \return timing information for the movie
        ///
        virtual const isx::TimingInfo &
        getTimingInfo() const
        {
            return m_timingInfo;
        }

        /// \return the total number of frames
        ///
        virtual isize_t
        getNumFrames() const
        {
            return m_timingInfo.getNumTimes();
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


        isx::TimingInfo m_timingInfo;                   //!< Movie timing information

        std::queue<FrameRequest>    m_frameRequestQueue;        //!< Frame request queue
        isx::Mutex                  m_frameRequestQueueMutex;   //!< Queue mutex

        typedef std::shared_ptr<MovieImpl> SpImpl_t;    //!< shared pointer
        typedef std::weak_ptr<MovieImpl> WpImpl_t;      //!< weak pointer
    };
}

#endif