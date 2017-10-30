#ifndef ISX_CELL_SET_UTILS_H
#define ISX_CELL_SET_UTILS_H

#include "isxCore.h"
#include "isxCellSet.h"
#include "isxImage.h"

namespace isx
{
    void
    cellSetToCellMap(
        const SpCellSet_t & inCellSet,
        bool inAcceptedCellsOnly,
        bool inNormalizeImages,
        SpImage_t outImage);

    void
    thresholdImage(
        SpImage_t inImage,
        float thresholdValue);

    void
        normalizeImageL1(
        SpImage_t inImage,
        float normalizationValue);

    void addImages(
        SpImage_t inImage1,
        SpImage_t inImage2,
        SpImage_t outImage);
}

#endif // define ISX_CELL_SET_UTILS_H