#include "isxExport.h"
#include "isxImage.h"
#include "isxException.h"
#include "isxCore.h"
#include "isxTiffBuffer.h"
#include "isxLogicalTrace.h"
#include "isxCellSet.h"
#include "isxTime.h"
#include "isxMovie.h"

#include <tiffio.h>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <limits>

#if ISX_OS_MACOS
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

namespace isx {


uint16_t
getTiffSampleFormat(DataType type)
{
	switch (type)
	{
	case DataType::F32:
		return SAMPLEFORMAT_IEEEFP;
		break;
	case DataType::U16:
	case DataType::U8:
	case DataType::RGB888:
		return SAMPLEFORMAT_UINT;
		break;
	default:
		ISX_THROW(isx::ExceptionUserInput, "Image's format is not supported.");
	}
}

void toTiffOut(TIFF *out, const SpImage_t & inImage)
{
	if (!inImage)
	{
		ISX_THROW(isx::ExceptionUserInput, "The image is invalid.");
	}

	uint32_t width = uint32_t(inImage->getSpacingInfo().getNumColumns());
	uint32_t height = uint32_t(inImage->getSpacingInfo().getNumRows());

	uint16_t sampleFormat = getTiffSampleFormat(inImage->getDataType());
	uint16_t bitsPerSample = uint16_t(getDataTypeSizeInBytes(inImage->getDataType()) * 8);

	TIFFSetField(out, TIFFTAG_IMAGEWIDTH, width);                             // set the width of the image
	TIFFSetField(out, TIFFTAG_IMAGELENGTH, height);                           // set the height of the image
	TIFFSetField(out, TIFFTAG_SAMPLESPERPIXEL, inImage->getNumChannels());    // set number of channels per pixel
	TIFFSetField(out, TIFFTAG_SAMPLEFORMAT, sampleFormat);             // how to interpret each data sample in a pixel
	TIFFSetField(out, TIFFTAG_BITSPERSAMPLE, bitsPerSample);                  // set the size of the channels
	TIFFSetField(out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);              // set the origin of the image.
	TIFFSetField(out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);             // set how the components of each pixel are stored (i.e. RGBRGBRGB or R plane, then G plane, then B plane ) 
	TIFFSetField(out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);          // set the color space of the image data

	isize_t linebytes = inImage->getRowBytes();

	// Allocating memory to store the pixels of current row
	TIFFBuffer buf(linebytes);

	// We set the strip size of the file to be size of one row of pixels
	TIFFSetField(out, TIFFTAG_ROWSPERSTRIP, 1);

	char * pixels = inImage->getPixels();
	for (uint32_t row = 0; row < height; row++)
	{
		std::memcpy(buf.get(), &pixels[row*linebytes], linebytes);
		if (TIFFWriteScanline(out, buf.get(), row, 0) < 0)
		{
			ISX_THROW(isx::ExceptionFileIO, "Error writing to output file.");
		}

	}
}

void toTiff(const std::string & inFileName, const SpCellSet_t & inSet)
{
	const isize_t numCells = inSet->getNumCells();

#if ISX_OS_MACOS
	const auto fd = creat(inFileName.c_str(), S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	TIFF *out = TIFFFdOpen(fd, inFileName.c_str(), "w");
#else
	TIFF *out = TIFFOpen(inFileName.c_str(), "w");
#endif

	if (!out)
	{
		ISX_THROW(isx::ExceptionFileIO, "Unable to open file for writing.");
	}

	for (isize_t cell = 0; cell < numCells; ++cell)
	{
		toTiffOut(out, inSet->getImage(cell));
		TIFFWriteDirectory(out);
	}

#if ISX_OS_MACOS
	TIFFCleanup(out);
	close(fd);
#else
	TIFFClose(out);
#endif

}

void toTiff(const std::string & inFileName, const SpMovie_t & inMovie)
{

}

void toTiff(const std::string & inFileName, const SpImage_t & inImage)
{
#if ISX_OS_MACOS
    const auto fd = creat(inFileName.c_str(), S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    TIFF *out = TIFFFdOpen(fd, inFileName.c_str(), "w");
#else
    TIFF *out = TIFFOpen(inFileName.c_str(), "w");
#endif

    if (!out)
    {
        ISX_THROW(isx::ExceptionFileIO, "Unable to open file for writing.");
    }

	toTiffOut(out, inImage);

#if ISX_OS_MACOS
    TIFFCleanup(out);
    close(fd);
#else
    TIFFClose(out);
#endif
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

} // namespace isx
