#ifndef ISX_CELL_SET_UTILS_H
#define ISX_CELL_SET_UTILS_H

#include "isxCoreFwd.h"

namespace isx
{
    /// Creates a cell map of all specified cell images in a cell set.
    ///
    /// This can normalize and threshold cell images.
    /// This can be set to only make a map of accepted cells in a cell set.
    ///
    /// \param  inCellSet               The cell set to extract cell images from.
    /// \param  inAcceptedCellsOnly     Boolean value to specify whether to only use accepted cells or not.
    /// \param  inNormalizeImages       Boolean value to specify whether to threshold and normalize each cell image.
    /// \return                         The cell map.
    SpImage_t
    cellSetToCellMap(
        const SpCellSet_t & inCellSet,
        bool inAcceptedCellsOnly,
        bool inNormalizeImages);
}

void
thresholdImage(
    isx::SpImage_t inImage,
    float thresholdValue);

void
normalizeImageL1(
    isx::SpImage_t inImage,
    float normalizationValue);

void addImages(
    isx::SpImage_t inImage1,
    isx::SpImage_t inImage2,
    isx::SpImage_t outImage);

void initializeWithZeros(
    isx::SpImage_t inImage);

#endif // define ISX_CELL_SET_UTILS_H