
#include "isxCellSetUtils.h"
#include "isxSpacingInfo.h"
#include "isxCellSet.h"
#include "isxImage.h"

#include <cstring>

namespace isx
{
    SpImage_t
    cellSetToCellMap(
        const SpCellSet_t & inCellSet,
        bool inAcceptedCellsOnly,
        bool inNormalizeImages)
    {
        SpImage_t outImage = std::make_shared<isx::Image>(inCellSet->getImage(0)->getSpacingInfo(), inCellSet->getImage(0)->getRowBytes(), inCellSet->getImage(0)->getNumChannels(), inCellSet->getImage(0)->getDataType());
        initializeWithZeros(outImage);

        for (isize_t i(0); i < inCellSet->getNumCells(); ++i)
        {
            CellSet::CellStatus status = inCellSet->getCellStatus(i);

            if ((inAcceptedCellsOnly && status == CellSet::CellStatus::UNDECIDED)
                || (status == CellSet::CellStatus::REJECTED))
            {
                continue;
            }

            SpImage_t im = inCellSet->getImage(i);

            if (inNormalizeImages)
            {
                ISX_ASSERT(im->getNumChannels() == 1);

                thresholdImage(im, 0.0);
                normalizeImageL1(im, 1.0);

                addImages(outImage, im, outImage);
            }
            else
            {
                addImages(outImage, im, outImage);
            }
        }

        return outImage;
    }
}

void
thresholdImage(
    isx::SpImage_t inImage,
    float thresholdValue)
{
    ISX_ASSERT(inImage->getDataType() == isx::DataType::F32);

    float * pixels = inImage->getPixelsAsF32();

    for (isx::isize_t i(0); i < inImage->getSpacingInfo().getTotalNumPixels(); ++i)
    {
        if (pixels[i] < thresholdValue)
        {
            pixels[i] = thresholdValue;
        }
    }
}

void
normalizeImageL1(
    isx::SpImage_t inImage,
    float normalizationValue)
{
    ISX_ASSERT(inImage->getDataType() == isx::DataType::F32);

    float * pixels = inImage->getPixelsAsF32();
    double sum = 0.0;

    for (isx::isize_t i(0); i < inImage->getSpacingInfo().getTotalNumPixels(); ++i)
    {
        sum += pixels[i];
    }

    ISX_ASSERT(sum != 0.0);
    normalizationValue /= (float)sum;

    for (isx::isize_t i(0); i < inImage->getSpacingInfo().getTotalNumPixels(); ++i)
    {
        pixels[i] *= normalizationValue;
    }
}

void addImages(
    isx::SpImage_t inImage1,
    isx::SpImage_t inImage2,
    isx::SpImage_t outImage)
{
    ISX_ASSERT(inImage1->getDataType() == isx::DataType::F32 &&
        inImage2->getDataType() == isx::DataType::F32 &&
        outImage->getDataType() == isx::DataType::F32);

    float * pixels1 = inImage1->getPixelsAsF32();
    float * pixels2 = inImage2->getPixelsAsF32();
    float * outPixels = outImage->getPixelsAsF32();

    ISX_ASSERT(inImage1->getSpacingInfo().getTotalNumPixels() == inImage2->getSpacingInfo().getTotalNumPixels());

    for (isx::isize_t i(0); i < inImage1->getSpacingInfo().getTotalNumPixels(); ++i)
    {
        outPixels[i] = pixels1[i] + pixels2[i];
    }
}

void initializeWithZeros(
    isx::SpImage_t inImage)
{
    ISX_ASSERT(inImage->getDataType() == isx::DataType::F32);

    float * pixels = inImage->getPixelsAsF32();

    std::memset(pixels, 0, sizeof(float) * inImage->getSpacingInfo().getTotalNumPixels());
}

