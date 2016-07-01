#include "isxDeltaFoverF.h"
#include "isxMovie.h"
#include "isxCoreFwd.h"
#include "isxProjectFile.h"

#include <iostream>
#include <limits>
#include <algorithm>

namespace isx
{

namespace
{
    void findMinMax(const std::vector<double> &input, double &min, double &max, size_t nLength)
    {
        auto localMinMax = std::minmax_element(input.begin(), input.end());
        min = std::min(min, *localMinMax.first);
        max = std::max(max, *localMinMax.second);
    }

    void scaleValues16(const std::vector<double> &input, double &min, double &max, std::vector<uint16_t> &output, size_t nLength)
    {
        if (min != max)
        {
            for (size_t i = 0; i < nLength; i++)
            {
                output[i] = uint16_t((4095 / (max - min)) * (input[i] - min));
            }
        }
    }

    void add_V(const std::vector<double> &inputA, uint16_t * inputB, std::vector<double> &output, size_t nLength)
    {
        for (size_t i = 0; i < nLength; i++)
        {
            output[i] = inputA[i] + double(inputB[i]);
        }
    }

    void subtract_V(uint16_t * inputA, const std::vector<double> &inputB, std::vector<double> &output, size_t nLength)
    {
        for (size_t i = 0; i < nLength; i++)
        {
            output[i] = double(inputA[i]) - inputB[i];
        }
    }

    void divide_V(const std::vector<double> &inputA, const std::vector<double> &inputB, std::vector<double> &output, size_t nLength)
    {
        for (size_t i = 0; i < nLength; i++)
        {
            if (inputB[i] != 0)
            {
                output[i] = double(inputA[i]) / inputB[i];
            }
            else
            {
                output[i] = 0;
            }

        }
    }

    void divide_C(const std::vector<double> &inputA, double num, std::vector<double> &output, size_t nLength)
    {
        if (num != 0)
        {
            for (size_t i = 0; i < nLength; i++)
            {
                output[i] = (double)inputA[i] / num;
            }
        }
        else
        {
            output.assign(nLength, 0);
        }
    }
} // namespace

AsyncTaskHandle::FinishedStatus applyDff(const SpMovie_t & inSource, const SpMovie_t & inDest, AsyncTaskHandle::CheckInCB_t inCheckInCB)
{
    float progress = 0.f;
    const size_t progressNumLoops = 3;
    size_t progressCurLoop = 0;
    
    if (inCheckInCB(progress))
    {
        return AsyncTaskHandle::FinishedStatus::CANCELLED;
    }
    if (inSource != nullptr && inDest != nullptr)
    {
        //calculate F0
        isize_t nFrameSize = inSource->getFrameSizeInBytes();
        isize_t nCols = inSource->getFrameWidth();
        isize_t nRows = inSource->getFrameHeight();
        isize_t nLength = nCols * nRows;
        isize_t nFrames = inSource->getNumFrames();

        std::vector<double> F0Image(nLength);
        for (isize_t i = 0; i < nFrames; i++)
        {
            auto f = inSource->getFrame(i);
            add_V(F0Image, f->getPixels(), F0Image, nLength);
            progress = ((float(i) / float(nFrames - 1)) + float(progressCurLoop)) / float(progressNumLoops);
            if (inCheckInCB(progress))
            {
                return AsyncTaskHandle::FinishedStatus::CANCELLED;
            }
        }
        ++progressCurLoop;

        divide_C(F0Image, static_cast<double>(nFrames), F0Image, nLength);

        //apply df/f
        std::vector<double> DFF(nLength);
        std::vector<double> temp(nLength);
        std::vector<uint16_t> output(nLength);

        double min = std::numeric_limits<double>::max();
        double max = std::numeric_limits<double>::min();
        for (isize_t i = 0; i < nFrames; i++)
        {
            auto f = inSource->getFrame(i);

            subtract_V(f->getPixels(), F0Image, DFF, nLength);
            divide_V(DFF, F0Image, temp, nLength);

            findMinMax(temp, min, max, nLength);
            progress = ((float(i) / float(nFrames - 1)) + float(progressCurLoop)) / float(progressNumLoops);
            if (inCheckInCB(progress))
            {
                return AsyncTaskHandle::FinishedStatus::CANCELLED;
            }
        }
        ++progressCurLoop;

        //output
        for (isize_t i = 0; i < nFrames; i++)
        {
            auto f = inSource->getFrame(i);

            subtract_V(f->getPixels(), F0Image, DFF, nLength);
            divide_V(DFF, F0Image, temp, nLength);

            scaleValues16(temp, min, max, output, nLength);

            inDest->writeFrame(i, &output[0], nFrameSize);
            progress = ((float(i) / float(nFrames - 1)) + float(progressCurLoop)) / float(progressNumLoops);
            if (inCheckInCB(progress))
            {
                return AsyncTaskHandle::FinishedStatus::CANCELLED;
            }
        }
        ++progressCurLoop;
    }
    inCheckInCB(1.f);
    return AsyncTaskHandle::FinishedStatus::COMPLETE;
}
} // namespace isx
