#include "isxExport.h"
#include "isxImage.h"
#include "isxException.h"
#include "isxCore.h"
#include "isxTiffBuffer.h"
#include "isxLogicalTrace.h"
#include "isxCellSet.h"
#include "isxTime.h"
#include "isxMovie.h"
#include "isxPathUtils.h"
#include "isxExportTiff.h"

#include <cstring>
#include <fstream>
#include <iomanip>
#include <limits>
#include <cmath>
#include <qglobal.h>
#include "QImage"
#include "QImageWriter"

#if ISX_OS_MACOS
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

namespace isx {

bool
writeIMULogicalTraces(
    std::ofstream & inStream,
    const std::vector<std::vector<std::vector<SpLogicalTrace_t>>> & inTraces,
    const std::vector<std::vector<std::string>> & inNames,
    const Time & inBaseTime,
    AsyncCheckInCB_t inCheckInCB)
{
    const size_t numSections = inTraces.size();
    ISX_ASSERT(numSections == 2);

    size_t numTraces = 0;
    for (isize_t s = 0; s < numSections; ++s)
    {
        numTraces += inTraces[s].size();
    }
    ISX_ASSERT(numTraces > 0);
    ISX_ASSERT(inNames.size() == inTraces.size());

    const int32_t maxDecimalsForDouble = std::numeric_limits<double>::digits10 + 1;

    // Write mag data in appended columns instead of rows
    // Section header
    inStream << "IMU Time (s)";
    for (const std::string & name : inNames[0])
    {
        inStream << ", " << name;
    }
    inStream << ", Mag Time (s)";
    for (const std::string & name : inNames[1])
    {
        inStream << ", " << name;
    }
    inStream << std::endl;

    std::vector<isx::Time> imuTimeVec;
    std::vector<std::vector<float>> imuTraceVec;
    std::vector<isx::Time> magTimeVec;
    std::vector<std::vector<float>> magTraceVec;

    // populate time vectors
    for (auto & dataset: inTraces[0][0])
    {
        for (auto & it: dataset->getValues())
        {
            imuTimeVec.push_back(it.first);
        }
    }
    for (auto & dataset: inTraces[1][0])
    {
        for (auto & it: dataset->getValues())
        {
            magTimeVec.push_back(it.first);
        }
    }
    // populate data vectors
    for (const auto & i : inTraces[0])
    {
        std::vector<float> trace;
        for (auto & dataset: i)
        {
            for (auto & it: dataset->getValues())
            {
                trace.push_back(it.second);
            }
        }
        imuTraceVec.push_back(trace);
    }
    for (const auto & i : inTraces[1])
    {
        std::vector<float> trace;
        for (auto & dataset: i)
        {
            for (auto & it: dataset->getValues())
            {
                trace.push_back(it.second);
            }
        }
        magTraceVec.push_back(trace);
    }

    // output csv one line at a time
    for (isize_t line = 0; line < imuTimeVec.size(); ++line)
    {
        // write imu time
        inStream << std::setprecision(maxDecimalsForDouble) << (imuTimeVec[line] - inBaseTime).toDouble();
        
        isize_t i = 0;
        // write imu data
        for (auto & tr : imuTraceVec)
        {
            // if acc, output in Gs
            if (i < 3)
            {
                inStream << ", " << tr[line] / 16384.f;
            }
            else
            {
                inStream << ", " << tr[line];
            }
            ++i;
        }

        // Write mag data
        if (line < magTimeVec.size())
        {
            // write mag time
            inStream << ", " << std::setprecision(maxDecimalsForDouble) << (magTimeVec[line] - inBaseTime).toDouble();
            for (auto & tr: magTraceVec)
            {
                inStream << ", " << tr[line];
            }
        }
        inStream << std::endl;
    }
    return false;
}

bool
writeLogicalTraces(
        std::ofstream & inStream,
        const std::vector<std::vector<SpLogicalTrace_t>> & inTraces,
        const std::vector<std::string> & inNames,
        const std::string & inNameHeader,
        const Time & inBaseTime,
        AsyncCheckInCB_t inCheckInCB)
{
    const size_t numTraces = inTraces.size();
    ISX_ASSERT(numTraces > 0);
    ISX_ASSERT(inNames.size() == numTraces);
    const size_t numSegments = inTraces.front().size();
    ISX_ASSERT(numSegments > 0);

    const int32_t maxDecimalsForDouble = std::numeric_limits<double>::digits10 + 1;
    const int32_t maxDecimalsForFloat = std::numeric_limits<float>::digits10 + 1;

    // we write all time/value pairs of each channel sequentially, so
    // we need a channel name column (which will be sorted)
    inStream << "Time (s), " << inNameHeader << ", Value\n";

    if (!inStream.good())
    {
        ISX_THROW(ExceptionFileIO, "Error writing to output file.");
    }

    // calculate the total number of lines for a progress update
    isize_t numLinesTotal = 0;
    for (size_t t = 0; t < numTraces; ++t)
    {
        for (size_t s = 0; s < numSegments; ++s)
        {
            numLinesTotal += inTraces[t][s]->getValues().size();
        }
    }

    isize_t numLinesWritten = 0;
    float progress = 0.f;
    bool cancelled = false;
    for (size_t t = 0; t < numTraces; ++t)
    {
        const std::string & name = inNames[t];
        for (size_t s = 0; s < numSegments; ++s)
        {
            for (const auto & tv : inTraces[t][s]->getValues())
            {
                inStream << std::setprecision(maxDecimalsForDouble) << (tv.first - inBaseTime).toDouble()
                         << ", " << name << ", "
                         << std::setprecision(maxDecimalsForFloat) << tv.second << "\n";

                if (!inStream.good())
                {
                    ISX_THROW(ExceptionFileIO, "Error writing to output file.");
                }

                ++numLinesWritten;
                progress = float(numLinesWritten) / float(numLinesTotal);
                cancelled = inCheckInCB(progress);
                if (cancelled)
                {
                    break;
                }
            }
        }
    }

    return cancelled;
}

bool writeTraces(
        std::ofstream & inStream,
        const std::vector<std::vector<SpFTrace_t>> & inTraces,
        const std::vector<std::string> & inNames,
        const std::vector<std::string> & inStatuses,
        const Time & inBaseTime,
        AsyncCheckInCB_t inCheckInCB)
{
    const size_t numSegments = inTraces.size();
    ISX_ASSERT(numSegments > 0);
    const size_t numTraces = inTraces.front().size();
    ISX_ASSERT(numTraces > 0);
    ISX_ASSERT(inNames.size() == numTraces);

    const int32_t maxDecimalsForDouble = std::numeric_limits<double>::digits10 + 1;
    const int32_t maxDecimalsForFloat = std::numeric_limits<float>::digits10 + 1;

    if (inStatuses.empty())
    {
        inStream << "Time (s)";
    }
    else
    {
        ISX_ASSERT(inStatuses.size() == numTraces);
        inStream << " ";
    }

    for (const auto & name : inNames)
    {
        inStream << ", " << name;
    }
    inStream << "\n";

    if (!inStatuses.empty())
    {
        inStream << "Time(s)/Cell Status";
        for (const auto & status : inStatuses)
        {
            inStream << ", " << status;
        }
        inStream << "\n";
    }

    if (!inStream.good())
    {
        ISX_THROW(ExceptionFileIO, "Error writing to output file.");
    }

    isize_t numLinesTotal = 0;
    for (const auto & segment : inTraces)
    {
        numLinesTotal += segment.front()->getTimingInfo().getNumTimes();
    }

    float progress = 0.f;
    isize_t numLinesWritten = 0;
    bool cancelled = false;

    for (const auto & segment : inTraces)
    {
        const SpFTrace_t & refTrace = segment.front();
        const TimingInfo & ti = refTrace->getTimingInfo();
        const isize_t numSamples = ti.getNumTimes();

        for (isize_t s = 0; s < numSamples; ++s)
        {
            const Time time = ti.convertIndexToStartTime(s);
            inStream << std::setprecision(maxDecimalsForDouble) << (time - inBaseTime).toDouble();

            for (isize_t t = 0; t < numTraces; ++t)
            {
                inStream << ", " << std::setprecision(maxDecimalsForFloat) << segment[t]->getValue(s);

                if (!inStream.good())
                {
                    ISX_THROW(ExceptionFileIO, "Error writing to output file.");
                }
            }
            inStream << "\n";

            ++numLinesWritten;
            progress = float(numLinesWritten) / float(numLinesTotal);
            cancelled = inCheckInCB(progress);
            if (cancelled)
            {
                break;
            }
        }
    }
    return cancelled;
}

bool 
toTiff(const std::string & inFileName, const std::vector<SpMovie_t> & inMovies, const bool inWriteInvalidFrames, const isize_t inMaxFrameIndex, AsyncCheckInCB_t & inCheckInCB)
{
    const std::string dirname = getDirName(inFileName);
    const std::string basename = getBaseName(inFileName);
    const std::string extension = getExtension(inFileName);

    auto cancelled = false;
    isize_t writtenFrames = 0;
    isize_t numFrames = 0;
    for (auto m : inMovies)
    {
        numFrames += m->getTimingInfo().getNumTimes();
    }

    size_t width = (numFrames > 10) ? (size_t(std::floor(std::log10(numFrames - 1)) + 1)) : (1);

    isize_t frameIndex = 0; // frame index of current movie
    isize_t mvCounter = 0; // movie counter for each 2^16-1 frames

    TiffExporter * out = new TiffExporter(inFileName, true); // for one movie - save to selected filess
    for (auto m : inMovies)
    {
        for (isize_t i = 0; i < m->getTimingInfo().getNumTimes(); ++i)
        {
            if (inWriteInvalidFrames || m->getTimingInfo().isIndexValid(i))
            {
                // if number of frames larger inMaxFrameIndex - increase file name and dump to new one
                if (frameIndex == inMaxFrameIndex) 
                {
                    mvCounter++;
                    frameIndex = 0;

                    const std::string fn = dirname + "/" + basename + "_" + convertNumberToPaddedString(mvCounter, width) + "." + extension;

                    delete out;
                    out = new TiffExporter(fn, true);
                }

                auto f = m->getFrame(i);
                auto& img = f->getImage();
                out->toTiffOut(&img, (inWriteInvalidFrames && !m->getTimingInfo().isIndexValid(i)));
                out->nextTiffDir();
                frameIndex++;
            }

            cancelled = inCheckInCB(float(++writtenFrames) / float(numFrames));
            if (cancelled)
            {
                break;
            }
        }            
        if (cancelled)
        {
            break;
        }
    }
    delete out;
    return cancelled;
}

void 
toTiff(const std::string & inFileName, const SpImage_t & inImage)
{
    toTiff(inFileName, inImage.get());
}

void 
toTiff(const std::string & inFileName, const Image * inImage)
{
    TiffExporter out(inFileName);
    out.toTiffOut(inImage);
}

void
toPng(const std::string & inFileName, const SpImage_t & inImage)
{
    ISX_ASSERT(inImage->getDataType() == DataType::U8);
    const isize_t numChannels = inImage->getNumChannels();
    ISX_ASSERT((numChannels == 1) || (numChannels == 3));

    QImage::Format format = (numChannels == 3) ? QImage::Format_RGB888 : QImage::Format_Grayscale8;

    QImage outImage = QImage(reinterpret_cast<const uchar*>(inImage->getPixels()),
            int(inImage->getWidth()), int(inImage->getHeight()), int(inImage->getRowBytes()), format);

    QImageWriter writer(inFileName.c_str());
    writer.setFormat("PNG");

    writer.write(outImage);
}

} // namespace isx
