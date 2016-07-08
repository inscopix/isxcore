#ifndef ISX_MOVIEDEFS_H
#define ISX_MOVIEDEFS_H

#include <functional>
#include <memory>

namespace isx {

    template <typename T>
    class VideoFrame;
    /// type for an nvista movie video frame
    ///
    typedef VideoFrame<uint16_t> U16VideoFrame_t;

    /// shared_ptr type for an nvista movie video frame
    ///
    typedef std::shared_ptr<U16VideoFrame_t> SpU16VideoFrame_t;

    /// Type of callback function to use to return video frames asynchronously
    ///
    typedef std::function<void(const SpU16VideoFrame_t & inVideoFrame)> MovieGetFrameCB_t;
}

#endif