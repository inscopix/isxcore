#include "isxAlgorithm.h"
#include "isxMovie.h"
#include "isxCoreFwd.h"
#include "isxProjectFile.h"

#include <iostream>
#include <limits>
#include <algorithm>

namespace isx
{
    Algorithm::Algorithm()
    {

    }

    Algorithm::Algorithm(const SpMovie_t & movie)
    {
        m_movie = movie;
    }

    bool Algorithm::IsValid()
    {
        return m_movie->isValid();
    }

    const SpMovie_t & Algorithm::getOutputMovie() const
    {
        return m_outputMovie;
    }

    void Algorithm::SetOutputMovie(const SpMovie_t & inMovie)
    {
        m_outputMovie = inMovie;
    }

    void Algorithm::FindMinMax(const std::vector<double> &input, double &min, double &max, size_t nLength)
    {
        auto localMinMax = std::minmax_element(input.begin(), input.end());
        min = std::min(min, *localMinMax.first);
        max = std::max(max, *localMinMax.second);
    }

    void Algorithm::ScaleValues16(const std::vector<double> &input, double &min, double &max, std::vector<uint16_t> &output, size_t nLength)
    {
        if (min != max)
        {
            for (size_t i = 0; i < nLength; i++)
            {
                output[i] = uint16_t((4095 / (max - min)) * (input[i] - min));
            }
        }
    }

    void Algorithm::Add_V(const std::vector<double> &inputA, uint16_t * inputB, std::vector<double> &output, size_t nLength)
    {
        for (size_t i = 0; i < nLength; i++)
        {
            output[i] = inputA[i] + double(inputB[i]);
        }
    }

    void Algorithm::Subtract_V(uint16_t * inputA, const std::vector<double> &inputB, std::vector<double> &output, size_t nLength)
    {
        for (size_t i = 0; i < nLength; i++)
        {
            output[i] = double(inputA[i]) - inputB[i];
        }
    }

    void Algorithm::Divide_V(const std::vector<double> &inputA, const std::vector<double> &inputB, std::vector<double> &output, size_t nLength)
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

    void Algorithm::Divide_C(const std::vector<double> &inputA, double num, std::vector<double> &output, size_t nLength)
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

    void Algorithm::ApplyApp()
    {
        if (m_movie != nullptr && m_outputMovie != nullptr)
        {
            //calculate F0
            size_t nFrameSize = m_movie->getFrameSizeInBytes();
            int nCols = m_movie->getFrameWidth();
            int nRows = m_movie->getFrameHeight();
            int nLength = nCols * nRows;
            int nFrames = m_movie->getNumFrames();

            std::vector<double> F0Image(nLength);
            for (int i = 0; i < nFrames; i++)
            {
                auto f = m_movie->getFrame(i);
                Add_V(F0Image, f->getPixels(), F0Image, nLength);
            }

            Divide_C(F0Image, nFrames, F0Image, nLength);

            //apply df/f
            std::vector<double> DFF(nLength);
            std::vector<double> temp(nLength);
            std::vector<uint16_t> output(nLength);

            double min = std::numeric_limits<double>::max();
            double max = std::numeric_limits<double>::min();
            for (int i = 0; i < nFrames; i++)
            {
                auto f = m_movie->getFrame(i);

                Subtract_V(f->getPixels(), F0Image, DFF, nLength);
                Divide_V(DFF, F0Image, temp, nLength);

                FindMinMax(temp, min, max, nLength);              
            }

            //output
            for (int i = 0; i < nFrames; i++)
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