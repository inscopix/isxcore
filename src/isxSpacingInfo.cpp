#include "isxSpacingInfo.h"
#include <cmath>

namespace isx
{

SpacingInfo::SpacingInfo()
    : m_numPixels(SizeInPixels_t(1440, 1080))
    , m_pixelSize(SizeInMicrons_t(Ratio(22, 10), Ratio(22, 10)))
    , m_topLeft(PointInMicrons_t(0, 0))
{
}

SpacingInfo::SpacingInfo(
    const SizeInPixels_t & numPixels,
    const SizeInMicrons_t & pixelSize,
    const PointInMicrons_t & topLeft)
    : m_numPixels(numPixels)
    , m_pixelSize(pixelSize)
    , m_topLeft(topLeft)
{
}

PointInMicrons_t
SpacingInfo::getTopLeft() const
{
    return m_topLeft;
}

PointInMicrons_t
SpacingInfo::getBottomRight() const
{
    return m_topLeft + getTotalSize();
}

SizeInMicrons_t
SpacingInfo::getPixelSize() const
{
    return m_pixelSize;
}

SizeInPixels_t
SpacingInfo::getNumPixels() const
{
    return m_numPixels;
}

isize_t
SpacingInfo::getNumRows() const
{
    return m_numPixels.getY();
}

isize_t
SpacingInfo::getNumColumns() const
{
    return m_numPixels.getX();
}

isize_t
SpacingInfo::getTotalNumPixels() const
{
    return m_numPixels.getX() * m_numPixels.getY();
}

SizeInMicrons_t
SpacingInfo::getTotalSize() const
{
    return m_pixelSize * m_numPixels;
}

PointInMicrons_t
SpacingInfo::convertPointInPixelsToMicrons(const PointInPixels_t & inPoint) const
{
    // Clamp pixel indices that exceed range and protect against subtraction from 0
    isize_t xPixels = inPoint.getX();
    isize_t xNumPixels = m_numPixels.getX();
    if (xPixels >= xNumPixels && xNumPixels > 0)
    {
        xPixels = xNumPixels - 1;
    }

    isize_t yPixels = inPoint.getY();
    isize_t yNumPixels = m_numPixels.getY();
    if (yPixels >= yNumPixels && yNumPixels > 0)
    {
        yPixels = yNumPixels - 1;
    }

    PointInPixels_t pointInPixels(xPixels, yPixels);

    // Multiply by pixel size and offset by center and top left
    PointInMicrons_t offset = m_pixelSize / SpatialVector<Ratio>(2, 2);
    return (m_pixelSize * pointInPixels) + offset + m_topLeft;
}

PointInPixels_t
SpacingInfo::convertPointInMicronsToPixels(const PointInMicrons_t & inPoint) const
{
    // Shift point by top left
    PointInMicrons_t pointInMicrons = inPoint - m_topLeft;

    // Divide by pixel size
    SpatialVector<Ratio> pixelsRatio = pointInMicrons / m_pixelSize;

    // Clamp and convert to positive indices
    Ratio xPixelsRatio = pixelsRatio.getX();
    Ratio xNumPixels = m_numPixels.getX();
    if (xPixelsRatio < 0)
    {
        xPixelsRatio = 0;
    }
    else if (xPixelsRatio >= xNumPixels && xNumPixels > 0)
    {
        xPixelsRatio = xNumPixels - 1;
    }

    Ratio yPixelsRatio = pixelsRatio.getY();
    Ratio yNumPixels = m_numPixels.getY();
    if (yPixelsRatio < 0)
    {
        yPixelsRatio = 0;
    }
    else if (yPixelsRatio >= yNumPixels && yNumPixels > 0)
    {
        yPixelsRatio = yNumPixels - 1;
    }

    isize_t xPixels = static_cast<isize_t>(std::floor(xPixelsRatio.toDouble()));
    isize_t yPixels = static_cast<isize_t>(std::floor(yPixelsRatio.toDouble()));

    return PointInPixels_t(xPixels, yPixels);
}

bool
SpacingInfo::operator ==(const SpacingInfo& other) const
{
    return (m_topLeft == other.m_topLeft)
        && (m_pixelSize == other.m_pixelSize)
        && (m_numPixels == other.m_numPixels);
}

void
SpacingInfo::serialize(std::ostream& strm) const
{
    strm << "SpacingInfo("
            << "NumPixels=" << m_numPixels << ", "
            << "PixelSize=" << m_pixelSize << ", "
            << "TopLeft=" << m_topLeft
         << ")";
}

} // namespace
