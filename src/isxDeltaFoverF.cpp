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

    bool DeltaFoverF::IsValid()
    {
        return m_movie->isValid();
    }

    const SpMovie_t & DeltaFoverF::getOutputMovie() const
    {
        return m_outputMovie;
    }

    void DeltaFoverF::SetOutputMovie(const SpMovie_t & inMovie)
    {
        m_outputMovie = inMovie;
    }

    void DeltaFoverF::FindMinMax(const std::vector<double> &input, double &min, double &max, size_t nLength)
    {
        auto localMinMax = std::minmax_element(input.begin(), input.end());
        min = std::min(min, *localMinMax.first);
        max = std::max(max, *localMinMax.second);
    }

    void DeltaFoverF::ScaleValues16(const std::vector<double> &input, double &min, double &max, std::vector<uint16_t> &output, size_t nLength)
    {
        if (min != max)
        {
            for (size_t i = 0; i < nLength; i++)
            {
                output[i] = uint16_t((4095 / (max - min)) * (input[i] - min));
            }
        }
    }

    void DeltaFoverF::Add_V(const std::vector<double> &inputA, uint16_t * inputB, std::vector<double> &output, size_t nLength)
    {
        for (size_t i = 0; i < nLength; i++)
        {
            output[i] = inputA[i] + double(inputB[i]);
        }
    }

    void DeltaFoverF::Subtract_V(uint16_t * inputA, const std::vector<double> &inputB, std::vector<double> &output, size_t nLength)
    {
        for (size_t i = 0; i < nLength; i++)
        {
            output[i] = double(inputA[i]) - inputB[i];
        }
    }

    void DeltaFoverF::Divide_V(const std::vector<double> &inputA, const std::vector<double> &inputB, std::vector<double> &output, size_t nLength)
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

    void DeltaFoverF::Divide_C(const std::vector<double> &inputA, double num, std::vector<double> &output, size_t nLength)
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

    void DeltaFoverF::ApplyApp()
    {
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
                Add_V(F0Image, f->getPixels(), F0Image, nLength);
            }

            Divide_C(F0Image, static_cast<double>(nFrames), F0Image, nLength);

            //apply df/f
            std::vector<double> DFF(nLength);
            std::vector<double> temp(nLength);
            std::vector<uint16_t> output(nLength);

            double min = std::numeric_limits<double>::max();
            double max = std::numeric_limits<double>::min();
            for (isize_t i = 0; i < nFrames; i++)
            {
                auto f = m_movie->getFrame(i);

                Subtract_V(f->getPixels(), F0Image, DFF, nLength);
                Divide_V(DFF, F0Image, temp, nLength);

                FindMinMax(temp, min, max, nLength);              
            }

            //output
            for (isize_t i = 0; i < nFrames; i++)
            {
                auto f = m_movie->getFrame(i);

                Subtract_V(f->getPixels(), F0Image, DFF, nLength);
                Divide_V(DFF, F0Image, temp, nLength);

                ScaleValues16(temp, min, max, output, nLength);

                m_outputMovie->writeFrame(i, &output[0], nFrameSize);
            }          
        }       
    }

}
