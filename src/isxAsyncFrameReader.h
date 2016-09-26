#ifndef ISX_ASYNC_FRAME_READER_H
#define ISX_ASYNC_FRAME_READER_H


#include "isxCore.h"
#include "isxMovie.h"
#include "isxMutex.h"

#include <memory>
#include <map>
#include <functional>

namespace isx
{

/// Utility class used by implementors of the Movie interface
class AsyncFrameReader : public std::enable_shared_from_this<AsyncFrameReader>
{
public:
    /// alias for getFrame callback to be passed into this frame reader
    using getFrameCB_t = std::function<SpVideoFrame_t()>;

    /// issues read frame request asynchronously
    /// \param inGetFrame getFrame callback provided by caller
    /// \param inCallback callback function to be called with frame
    void 
    getFrameAsync(getFrameCB_t inGetFrame, MovieGetFrameCB_t inCallback);

    /// cancels all pending read requests
    void 
    cancelPendingReads();

private:
    SpAsyncTaskHandle_t
    unregisterReadRequest(uint64_t inReadRequestId);

    uint64_t                                m_readRequestCount = 0;
    isx::Mutex                              m_pendingReadsMutex;
    std::map<uint64_t, SpAsyncTaskHandle_t> m_pendingReads;

};
} // namespace isx
#endif // def ISX_ASYNC_FRAME_READER_H
