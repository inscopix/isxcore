#include "isxVesselCorrelations.h"

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

namespace isx
{

VesselCorrelations::VesselCorrelations() {}

VesselCorrelations::VesselCorrelations(
    const SizeInPixels_t inSizeInPixels,
    const float inMin,
    const float inMax)
    : m_sizeInPixels(inSizeInPixels)
    , m_min(inMin)
    , m_max(inMax)
{
    size_t numPixels = getTotalNumPixels() * 3;
    m_values.reset(new float[numPixels]);  
    std::memset(m_values.get(), 0, sizeof(float)*numPixels);     
}

const SizeInPixels_t & VesselCorrelations::getNumPixels() const
{
    return m_sizeInPixels;
}

size_t VesselCorrelations::getTotalNumPixels() const
{
    return m_sizeInPixels.getWidth() * m_sizeInPixels.getHeight();
}

float *
VesselCorrelations::getValues()
{
    if (m_values)
    {
        return &m_values[0];
    }
    return 0;
}

float *
VesselCorrelations::getValues(const int inOffset)
{
    float * data = getValues();
    if (data)
    {
        size_t offset = (inOffset + 1) * getTotalNumPixels();
        data += offset;
        return data;
    }
    return 0;
}

void VesselCorrelations::setValues(const int inOffset, const float * inData)
{
    float * data = getValues(inOffset);
    std::memcpy(data, inData, getTotalNumPixels() * sizeof(float));
}

SpImage_t VesselCorrelations::getHeatmap(const int inOffset)
{
    const float * data = getValues(inOffset);
    SpImage_t image(new Image(SpacingInfo(m_sizeInPixels), m_sizeInPixels.getWidth() * sizeof(float), 1, DataType::F32));
    std::memcpy(image->getPixelsAsF32(), data, getTotalNumPixels() * sizeof(float));
    return image;
}

SpImage_t VesselCorrelations::getHeatmaps()
{
    const bool rowWise = (m_sizeInPixels.getWidth() >= m_sizeInPixels.getHeight()); // indicate heatmaps are placed in rows or cols per frame
    const SizeInPixels_t dstSize = computeTriptychSize(m_sizeInPixels);
    const int width = static_cast<int>(m_sizeInPixels.getWidth());
    const int height = static_cast<int>(m_sizeInPixels.getHeight());
    cv::Mat dst(static_cast<int>(dstSize.getHeight()), static_cast<int>(dstSize.getWidth()), CV_32F);
    for (int offset = -1; offset <= 1; offset++)
    {
        const cv::Mat src(height, width, CV_32F, getValues(offset));
        if (rowWise)
        {
            src.copyTo(dst(cv::Rect(
                0, height * (offset + 1), width, height
            )));
        }
        else
        {
            src.copyTo(dst(cv::Rect(
                width * (offset + 1), 0, width, height
            )));
        }
    }

    SpImage_t image(new Image(SpacingInfo(dstSize), dstSize.getWidth() * sizeof(float), 1, DataType::F32));
    std::memcpy(image->getPixelsAsF32(), dst.ptr(), 3 * getTotalNumPixels() * sizeof(float));
    return image;
}

void VesselCorrelations::calculateMinMax()
{
    m_min = std::numeric_limits<float>::max();
    m_max = std::numeric_limits<float>::min();

    const size_t numPixels = getTotalNumPixels() * 3;
    const float * values = getValues();
    bool allNan = true;
    for (size_t p = 0; p < numPixels; p++)
    {
        if (!std::isnan(values[p]))
        {
            allNan = false;
            m_min = std::min(m_min, values[p]);
            m_max = std::max(m_max, values[p]);

        }
    }

    if (allNan)
    {
        m_min = std::numeric_limits<float>::quiet_NaN();
        m_max = std::numeric_limits<float>::quiet_NaN();
    }
}

float VesselCorrelations::getMin()
{
    return m_min;
}

float VesselCorrelations::getMax()
{
    return m_max;
}

Contours_t mapBoxToMinimunRect(const Contour_t & inBoundingBox)
{
    std::vector<cv::Point> coordinates;
    for (size_t k = 0; k < 4; k++)
    {
        coordinates.push_back(cv::Point(int(inBoundingBox[k].getX()),int(inBoundingBox[k].getY())));
    }

    Contours_t outBoxes;
    const cv::Rect minCartesianRect = cv::boundingRect(coordinates);
    const bool rowWise = minCartesianRect.width >= minCartesianRect.height;
    for (int offset = -1; offset <= 1; offset++)
    {
        Contour_t outBox;
        for (size_t k = 0; k < 4; k++)
        {
            const PointInPixels_t point(
                inBoundingBox[k].getX() - minCartesianRect.tl().x + minCartesianRect.width * (rowWise ? 0 : (offset + 1)),
                inBoundingBox[k].getY() - minCartesianRect.tl().y + minCartesianRect.height * (rowWise ? (offset + 1) : 0)
            ); 
            outBox.push_back(point);
        }
        outBoxes.push_back(outBox);
    }

    return outBoxes;
}

PointInPixels_t computeCenterOfBoundingBox(const Contour_t & inBoundingBox)
{
    int64_t xmin = std::numeric_limits<int64_t>::max();
    int64_t xmax = std::numeric_limits<int64_t>::min();
    int64_t ymin = std::numeric_limits<int64_t>::max();
    int64_t ymax = std::numeric_limits<int64_t>::min();
    for (const auto point : inBoundingBox)
    {
        xmin = std::min(xmin, point.getX());
        xmax = std::max(xmax, point.getX());
        ymin = std::min(ymin, point.getY());
        ymax = std::max(ymax, point.getY());
    }
    return PointInPixels_t((xmin + xmax) / 2, (ymin + ymax) / 2);
}

SizeInPixels_t computeTriptychSize(const SizeInPixels_t & inCorrelationSize)
{
    if (inCorrelationSize.getWidth() >= inCorrelationSize.getHeight())
    {
        // arrange heatmaps in rows
        return SizeInPixels_t(inCorrelationSize.getWidth(), inCorrelationSize.getHeight() * 3);
    }
    else
    {
        // arrange heatmaps in cols
        return SizeInPixels_t(inCorrelationSize.getWidth() * 3, inCorrelationSize.getHeight());
    }
}
} // namespace isx
