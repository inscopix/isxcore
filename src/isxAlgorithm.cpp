#include "isxAlgorithm.h"
#include "isxMovie.h"
#include "H5Cpp.h"
#include "isxCoreFwd.h"
#include "isxProjectFile.h"

#include <iostream>

namespace isx
{
    Algorithm::Algorithm()
    {

    }

    Algorithm::Algorithm(SpMovie_t movie)
    {
        m_Movie = movie;
    }

    bool Algorithm::IsValid()
    {
        return m_Movie->isValid();
    }

    std::string Algorithm::GetOutputFileName()
    {
        return m_OutputFileName;
    }

    void Algorithm::SetOutputFileName(std::string outputFile)
    {
        m_OutputFileName = outputFile;
    }

    void Algorithm::FindMinMax(const std::vector<double> &input, double &min, double &max, size_t nLength)
    {
        for (int i = 0; i < nLength; i++)
        {
            if (input[i] < min)
            {
                min = input[i];
            }

            if (input[i] > max)
            {
                max = input[i];
            }
        }
    }

    void Algorithm::ScaleValues16(const std::vector<double> &input, double &min, double &max, std::vector<uint16_t> &output, size_t nLength)
    {
        if (min != max)
        {
            for (int i = 0; i < nLength; i++)
            {
                output[i] = uint16_t((4095 / (max - min)) * (input[i] - min));
            }
        }
    }

    void Algorithm::Add_V(const std::vector<double> &inputA, const std::vector<uint16_t> &inputB, std::vector<double> &output, size_t nLength)
    {
        for (int i = 0; i < nLength; i++)
        {
            output[i] = inputA[i] + double(inputB[i]);
        }
    }

    void Algorithm::Subtract_V(const std::vector<uint16_t> &inputA, const std::vector<double> &inputB, std::vector<double> &output, size_t nLength)
    {
        for (int i = 0; i < nLength; i++)
        {
            output[i] = double(inputA[i]) - inputB[i];
        }
    }

    void Algorithm::Divide_V(const std::vector<double> &inputA, const std::vector<double> &inputB, std::vector<double> &output, size_t nLength)
    {
        for (int i = 0; i < nLength; i++)
        {
            if (inputA[i] != 0)
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
            for (int i = 0; i < nLength; i++)
            {
                output[i] = (double)inputA[i] / num;
            }
        }
        // else error
    }

    void Algorithm::ApplyApp()
    {
        if (m_Movie != nullptr && m_OutputFileName != "")
        {
            //calculate F0
            size_t nFrameSize = m_Movie->getFrameSizeInBytes();
            int nCols = m_Movie->getFrameWidth();
            int nRows = m_Movie->getFrameHeight();
            int nLength = nCols * nRows;
            int nFrames = m_Movie->getNumFrames();

            std::vector<uint16_t> buffer(nLength);
            //std::vector<double> frames(nFrameSize);
            std::vector<double> F0Image(nLength);
            for (int i = 0; i < nFrames; i++)
            {
                m_Movie->getFrame(i, &buffer[0], nFrameSize);

                Add_V(F0Image, buffer, F0Image, nLength);
            }

            Divide_C(F0Image, nFrames, F0Image, nLength);

            isx::SpProjectFile_t outputFile = std::make_shared<isx::ProjectFile>(m_OutputFileName);
            isx::Movie outputMovie(outputFile->getHdf5FileHandle(), "/images", nFrames, nCols, nRows);

            //apply df/f
            std::vector<double> DFF(nLength);
            std::vector<double> temp(nLength);
            std::vector<uint16_t> output(nLength);

            double min = 0;
            double max = 0;
            for (int i = 0; i < nFrames; i++)
            {
                m_Movie->getFrame(i, &buffer[0], nFrameSize);

                Subtract_V(buffer, F0Image, DFF, nLength);
                Divide_V(DFF, F0Image, temp, nLength);

                FindMinMax(temp, min, max, nLength);              
            }

            //output
            for (int i = 0; i < nFrames; i++)
            {
                m_Movie->getFrame(i, &buffer[0], nFrameSize);

                Subtract_V(buffer, F0Image, DFF, nLength);
                Divide_V(DFF, F0Image, temp, nLength);

                ScaleValues16(temp, min, max, output, nLength);

                outputMovie.writeFrame(i, &output[0], nFrameSize);
            }          
        }       
    }

}