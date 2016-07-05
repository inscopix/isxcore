#ifndef ISX_MOVIEIMPL_H
#define ISX_MOVIEIMPL_H


#include "isxIoQueue.h"
#include <queue>
#include "isxMovie.h"     // For MovieGetFrameCB_t & SpU16VideoFrame_t


namespace isx 
{
    class MovieImpl : public std::enable_shared_from_this<MovieImpl>
    {
    public: 
        virtual SpU16VideoFrame_t getFrame(isize_t inFrameNumber) = 0;


        SpU16VideoFrame_t
            getFrame(const Time & inTime)
        {
            isize_t frameNumber = m_timingInfo.convertTimeToIndex(inTime);
            return getFrame(frameNumber);
        }

        void
            getFrameAsync(isize_t inFrameNumber, MovieGetFrameCB_t inCallback)
        {
            {
                isx::ScopedMutex locker(m_frameRequestQueueMutex, "getFrameAsync");
                m_frameRequestQueue.push(FrameRequest(inFrameNumber, inCallback));
            }
            processFrameQueue();
        }

        void
            getFrameAsync(const Time & inTime, MovieGetFrameCB_t inCallback)
        {
            isize_t frameNumber = m_timingInfo.convertTimeToIndex(inTime);
            return getFrameAsync(frameNumber, inCallback);
        }

        double
            getDurationInSeconds() const
        {
            return m_timingInfo.getDuration().toDouble();
        }

        const isx::TimingInfo &
            getTimingInfo() const
        {
            return m_timingInfo;
        }

        isize_t
            getNumFrames() const
        {
            return m_timingInfo.getNumTimes();
        }

    protected:
        class FrameRequest
        {
        public:
            FrameRequest(isize_t inFrameNumber, MovieGetFrameCB_t inCallback)
                : m_frameNumber(inFrameNumber)
                , m_callback(inCallback) {}

            isize_t             m_frameNumber;
            MovieGetFrameCB_t   m_callback;
        };

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

        void
            purgeFrameQueue()
        {
            isx::ScopedMutex locker(m_frameRequestQueueMutex, "getFrameAsync");
            while (!m_frameRequestQueue.empty())
            {
                m_frameRequestQueue.pop();
            }
        }


        isx::TimingInfo m_timingInfo;

        std::queue<FrameRequest>    m_frameRequestQueue;
        isx::Mutex                  m_frameRequestQueueMutex;

        typedef std::shared_ptr<MovieImpl> SpImpl_t;
        typedef std::weak_ptr<MovieImpl> WpImpl_t;
    };
}

#endif