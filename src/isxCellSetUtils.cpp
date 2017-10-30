
#include "isxCellSetUtils.h"
#include "isxSpacingInfo.h"

#include <cstring>

namespace isx
{
    void
    cellSetToCellMap(
        const SpCellSet_t & inCellSet,
        bool inAcceptedCellsOnly,
        bool inNormalizeImages,
        SpImage_t outImage)
    {
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
    }

    void
    thresholdImage(
        SpImage_t inImage,
        float thresholdValue)
    {
        float * pixels = inImage->getPixelsAsF32();

        for (isize_t i(0); i < inImage->getSpacingInfo().getTotalNumPixels(); ++i)
        {
            if (pixels[i] < thresholdValue)
            {
                pixels[i] = thresholdValue;
            }
        }
    }

    void
    normalizeImageL1(
        SpImage_t inImage,
        float normalizationValue)
    {
        float * pixels = inImage->getPixelsAsF32();
        double sum = 0.0;

        for (isize_t i(0); i < inImage->getSpacingInfo().getTotalNumPixels(); ++i)
        {
            sum += pixels[i];
        }

        ISX_ASSERT(sum != 0.0);

        for (isize_t i(0); i < inImage->getSpacingInfo().getTotalNumPixels(); ++i)
        {
            pixels[i] /= (float)sum;
            pixels[i] *= normalizationValue;
        }
    }

    void addImages(
        SpImage_t inImage1,
        SpImage_t inImage2,
        SpImage_t outImage)
    {
        float * pixels1 = inImage1->getPixelsAsF32();
        float * pixels2 = inImage2->getPixelsAsF32();
        float * outPixels = outImage->getPixelsAsF32();

        ISX_ASSERT(inImage1->getSpacingInfo().getTotalNumPixels() == inImage2->getSpacingInfo().getTotalNumPixels());

        for (isize_t i(0); i < inImage1->getSpacingInfo().getTotalNumPixels(); ++i)
        {
            outPixels[i] = pixels1[i] + pixels2[i];
        }
    }

    void initializeWithZeros(
        SpImage_t inImage)
    {
        float * pixels = inImage->getPixelsAsF32();
        
        std::memset(pixels, 0, sizeof(float) * inImage->getSpacingInfo().getTotalNumPixels());
    }
}

