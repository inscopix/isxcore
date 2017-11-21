
#include "isxCellSetUtils.h"
#include "isxSpacingInfo.h"
#include "isxCellSet.h"
#include "isxImage.h"
#include "isxLog.h"

#include <cstring>
#include <algorithm>

namespace {

void
addImages(
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

void
initializeWithZeros(
    isx::SpImage_t inImage)
{
    ISX_ASSERT(inImage->getDataType() == isx::DataType::F32);

    float * pixels = inImage->getPixelsAsF32();

    // std::memset(pixels, 0, sizeof(float) * inImage->getSpacingInfo().getTotalNumPixels());
    for (isx::isize_t i(0); i < inImage->getSpacingInfo().getTotalNumPixels(); ++i)
    {
        pixels[i] = 0.0f;
    } 
 }

float
pixSum(isx::SpImage_t imCopy)
{
    float * _pixels = imCopy->getPixelsAsF32();
    float _pixSum = 0.0f;
    for (isx::isize_t _i(0); _i < imCopy->getSpacingInfo().getTotalNumPixels(); ++_i)
    {
        _pixSum += _pixels[_i];
    }
    return _pixSum;    
}

}


namespace isx {

isx::SpImage_t
normalizeAndThresholdImage(isx::SpImage_t inImage, float inNormalizedThreshold)
{
    ISX_ASSERT(inImage->getDataType() == isx::DataType::F32);
    ISX_ASSERT(inNormalizedThreshold >= 0.0);
    ISX_ASSERT(inNormalizedThreshold <= 1.0);

    // copy image so the origin is not modified
    isx::SpImage_t imgCopy(inImage);

    float * pixels = imgCopy->getPixelsAsF32();
    isx::isize_t numPixels = imgCopy->getSpacingInfo().getTotalNumPixels();

    // identify image max
    float maxVal = 0.0f;
    for (isx::isize_t i(0); i < numPixels; ++i)
    {    
        maxVal = std::max(maxVal, pixels[i]);
    }

    if (maxVal <= 0.0)
    {
        maxVal = 1.0;
    }

    for (isx::isize_t i(0); i < numPixels; ++i)
    {
        pixels[i] /= maxVal;

        if (pixels[i] < inNormalizedThreshold)
        {            
            pixels[i] = 0.0;
        }        
    }

    return imgCopy;
}

SpImage_t
cellSetToCellMap(
    const SpCellSet_t & inCellSet,
    bool inAcceptedCellsOnly,
    bool inNormalizeImages,
    float inNormalizedThreshold)
{
    //SpImage_t outImage = std::make_shared<isx::Image>(inCellSet->getImage(0)->getSpacingInfo(), inCellSet->getImage(0)->getRowBytes(), inCellSet->getImage(0)->getNumChannels(), inCellSet->getImage(0)->getDataType());
    SpImage_t outImage(inCellSet->getImage(0));
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
        ISX_ASSERT(im->getNumChannels() == 1);

        if (inNormalizeImages)
        {            
            SpImage_t threshImg = normalizeAndThresholdImage(im, inNormalizedThreshold);
            addImages(outImage, threshImg, outImage);
        }
        else
        {
            addImages(outImage, im, outImage);
        }
    }

    return outImage;
}

template <typename T>
void
getImageMinMaxInternal(
        const Image & inImage,
        const isize_t inNumChannels,
        const float inNormalizer,
        float & outMin,
        float & outMax)
{
    auto min = std::numeric_limits<T>::max();
    auto max = std::numeric_limits<T>::lowest();
    auto pixels = reinterpret_cast<const T *>(inImage.getPixels());
    const isize_t numPixels = inNumChannels * inImage.getSpacingInfo().getTotalNumPixels();
    for (isize_t i(0); i < numPixels; ++i)
    {
        min = std::min<T>(min, pixels[i]);
        max = std::max<T>(max, pixels[i]);
    }
    outMin = float(min) / inNormalizer;
    outMax = float(max) / inNormalizer;
}

void
getImageMinMax(
    const Image & inImage, 
    float & outMin, 
    float & outMax)
{
    DataType dt = inImage.getDataType();
    switch (dt)
    {
    case DataType::U16:
        getImageMinMaxInternal<uint16_t>(inImage, 1, 1.0f, outMin, outMax);
        break;

    case DataType::F32:
        getImageMinMaxInternal<float>(inImage, 1, 1.0f, outMin, outMax);
        break;

    case DataType::U8:
        getImageMinMaxInternal<uint8_t>(inImage, 1, 255.f, outMin, outMax);
        break;

    case DataType::RGB888:
        getImageMinMaxInternal<uint8_t>(inImage, 3, 255.f, outMin, outMax);
        break;

    default:
        ISX_THROW(isx::ExceptionFileIO, "Unsupported datatype: ", dt);
        break;
    }
}

SpImage_t
convertImageF32toU8(
    const isx::SpImage_t & inImage)
{
    ISX_ASSERT(inImage->getDataType() == DataType::F32);

    isx::SpImage_t outImage = std::make_shared<isx::Image>(inImage->getSpacingInfo(), inImage->getWidth() * inImage->getNumChannels() * getDataTypeSizeInBytes(DataType::U8), inImage->getNumChannels(), DataType::U8);

    float * inPixels = inImage->getPixelsAsF32();
    uint8_t * outPixels = outImage->getPixelsAsU8();

    float min, max;
    isx::getImageMinMax(*inImage, min, max);

    ISX_ASSERT(max != min);

    const float normalizationFactor = 255.f / (max - min);

    for (isize_t i(0); i < inImage->getSpacingInfo().getTotalNumPixels(); ++i)
    {
        outPixels[i] = uint8_t((inPixels[i] - min) * normalizationFactor);
    }
        
    return outImage;
}

}  // namespace isx