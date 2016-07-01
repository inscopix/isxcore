#include "isxDeltaFoverF.h"
#include "isxMovie.h"
#include "isxCoreFwd.h"
#include "isxProjectFile.h"

#include <iostream>
#include <limits>
#include <algorithm>

namespace isx
{
    DeltaFoverF::DeltaFoverF()
    {

    }

    DeltaFoverF::DeltaFoverF(const SpMovie_t & movie)
    {
        m_movie = movie;
    }

    bool DeltaFoverF::isValid()
    {
        return m_movie->isValid();
    }

    const SpMovie_t & DeltaFoverF::getOutputMovie() const
    {
        return m_outputMovie;
    }

    void DeltaFoverF::setOutputMovie(const SpMovie_t & inMovie)
    {
        m_outputMovie = inMovie;
    }

    void DeltaFoverF::findMinMax(const std::vector<double> &input, double &min, double &max, size_t nLength)
    {
        auto localMinMax = std::minmax_element(input.begin(), input.end());
        min = std::min(min, *localMinMax.first);
        max = std::max(max, *localMinMax.second);
    }

    void DeltaFoverF::scaleValues16(const std::vector<double> &input, double &min, double &max, std::vector<uint16_t> &output, size_t nLength)
    {
        if (min != max)
        {
            for (size_t i = 0; i < nLength; i++)
            {
                output[i] = uint16_t((4095 / (max - min)) * (input[i] - min));
            }
        }
    }

    void DeltaFoverF::add_V(const std::vector<double> &inputA, uint16_t * inputB, std::vector<double> &output, size_t nLength)
    {
        for (size_t i = 0; i < nLength; i++)
        {
            output[i] = inputA[i] + double(inputB[i]);
        }
    }

    void DeltaFoverF::subtract_V(uint16_t * inputA, const std::vector<double> &inputB, std::vector<double> &output, size_t nLength)
    {
        for (size_t i = 0; i < nLength; i++)
        {
            output[i] = double(inputA[i]) - inputB[i];
        }
    }

    void DeltaFoverF::divide_V(const std::vector<double> &inputA, const std::vector<double> &inputB, std::vector<double> &output, size_t nLength)
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

    void DeltaFoverF::divide_C(const std::vector<double> &inputA, double num, std::vector<double> &output, size_t nLength)
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

    AsyncTaskHandle::FinishedStatus DeltaFoverF::run(AsyncTaskHandle::CheckInCB_t inCheckInCB)
    {
        float progress = 0.f;
        const size_t progressNumLoops = 3;
        size_t progressCurLoop = 0;
        m_checkInCB = inCheckInCB;
        
        if (m_checkInCB(progress))
        {
            return AsyncTaskHandle::FinishedStatus::CANCELLED;
        }
        if (m_movie != nullptr && m_outputMovie != nullptr)
        {
            //calculate F0
            isize_t nFrameSize = m_movie->getFrameSizeInBytes();
            isize_t nCols = m_movie->getFrameWidth();
            isize_t nRows = m_movie->getFrameHeight();
            isize_t nLength = nCols * nRows;
            isize_t nFrames = m_movie->getNumFrames();

            std::vector<double> F0Image(nLength);
            for (isize_t i = 0; i < nFrames; i++)
            {
                auto f = m_movie->getFrame(i);
                add_V(F0Image, f->getPixels(), F0Image, nLength);
                progress = ((float(i) / float(nFrames - 1)) + float(progressCurLoop)) / float(progressNumLoops);
                if (m_checkInCB(progress))
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
                auto f = m_movie->getFrame(i);

                subtract_V(f->getPixels(), F0Image, DFF, nLength);
                divide_V(DFF, F0Image, temp, nLength);

                findMinMax(temp, min, max, nLength);
                progress = ((float(i) / float(nFrames - 1)) + float(progressCurLoop)) / float(progressNumLoops);
                if (m_checkInCB(progress))
                {
                    return AsyncTaskHandle::FinishedStatus::CANCELLED;
                }
            }
            ++progressCurLoop;

            if (m_checkInCB(0.66f))
            {
                return AsyncTaskHandle::FinishedStatus::CANCELLED;
            }

            //output
            for (isize_t i = 0; i < nFrames; i++)
            {
                auto f = m_movie->getFrame(i);

                subtract_V(f->getPixels(), F0Image, DFF, nLength);
                divide_V(DFF, F0Image, temp, nLength);

                scaleValues16(temp, min, max, output, nLength);

                m_outputMovie->writeFrame(i, &output[0], nFrameSize);
                progress = ((float(i) / float(nFrames - 1)) + float(progressCurLoop)) / float(progressNumLoops);
                if (m_checkInCB(progress))
                {
                    return AsyncTaskHandle::FinishedStatus::CANCELLED;
                }
            }
            ++progressCurLoop;
        }
        m_checkInCB(1.f);
        return AsyncTaskHandle::FinishedStatus::COMPLETE;
    }
}
