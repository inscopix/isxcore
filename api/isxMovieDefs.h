#ifndef ISX_MOVIEDEFS_H
#define ISX_MOVIEDEFS_H

#include <functional>
#include <memory>

namespace isx {

    template <typename T>
    class VideoFrame;

    /// A video frame containing uint16 pixels.
    ///
    typedef VideoFrame<uint16_t> U16VideoFrame_t;

    /// A shared pointer to a video frame containing uint16 pixels.
    ///
    typedef std::shared_ptr<U16VideoFrame_t> SpU16VideoFrame_t;

    /// A callback function to use to get a video frame containing uint16 pixels.
    ///
    typedef std::function<void(const SpU16VideoFrame_t & inVideoFrame)> MovieGetU16FrameCB_t;

    /// A video frame containing uint16 pixels.
    ///
    typedef VideoFrame<float> F32VideoFrame_t;

    /// A shared pointer to a video frame containing uint16 pixels.
    ///
    typedef std::shared_ptr<F32VideoFrame_t> SpF32VideoFrame_t;

    /// A callback function to use to get a video frame containing uint16 pixels.
    ///
    typedef std::function<void(const SpF32VideoFrame_t & inVideoFrame)> MovieGetF32FrameCB_t;
}

#endif
