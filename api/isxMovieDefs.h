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

    /// A video frame containing float pixels.
    ///
    typedef VideoFrame<float> F32VideoFrame_t;

    /// A shared pointer to a video frame containing float pixels.
    ///
    typedef std::shared_ptr<F32VideoFrame_t> SpF32VideoFrame_t;

    /// The type of getFrame call back templated by frame type
    template <typename FrameType> using MovieGetFrameCB_t =
        std::function<void(const std::shared_ptr<FrameType> & inVideoFrame)>;

    /// A callback function to use to get a video frame containing uint16 pixels.
    ///
    typedef MovieGetFrameCB_t<U16VideoFrame_t> MovieGetU16FrameCB_t;

    /// A callback function to use to get a video frame containing float pixels.
    ///
    typedef MovieGetFrameCB_t<F32VideoFrame_t> MovieGetF32FrameCB_t;
}

#endif
