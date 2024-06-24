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
    /// \param  inNormalizedThreshold   If inNormalizeImages == true, threshold the image after normalization,
    ///                                 zeroing out any values that are less than inNormalizeThreshold, which should
    ///                                 be between 0 and 1.
    /// \return                         The cell map.
    SpImage_t
    cellSetToCellMap(
        const SpCellSet_t & inCellSet,
        bool inAcceptedCellsOnly,
        bool inNormalizeImages,
        float inNormalizedThreshold = 0.0f);

    /// Gets the min and max values from an image.
    ///
    /// \param inImage  The input image.
    /// \param outMin   The min value.
    /// \param outMax   The max value.
    void
    getImageMinMax(
        const Image & inImage, 
        float & outMin, 
        float & outMax);

    /// Converts and rescales an image with DataType::F32 to an image with DataType::U8 
    /// so that the min image value is mapped to 0 and the max image value is mapped to 255.
    /// \param  inImage         The image to convert.
    /// \return                 The converted image.
    SpImage_t
    convertImageF32toU8(
        const SpImage_t & inImage);

    /// Normalizes an image by dividing by the maximum value, then thresholds the normalized image.
    ///
    /// \param inImage              The input image to threshold. Input image is not modified
    /// \param inNormalizedThreshold  The threshold value, between 0 and 1. All pixel values less than this value are set to zero.
    ///
    SpImage_t
    normalizeAndThresholdImage(isx::SpImage_t inImage, float inNormalizedThreshold);
}

#endif // define ISX_CELL_SET_UTILS_H
