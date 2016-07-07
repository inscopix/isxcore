#include "isxSpacingInfo.h"
#include <algorithm>
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
    // Clamp pixel indices that exceed range
    isize_t xPixels = std::min(inPoint.getX(), m_numPixels.getX() - 1);
    isize_t yPixels = std::min(inPoint.getY(), m_numPixels.getY() - 1);
    PointInPixels_t pointInPixels(xPixels, yPixels);

    // Scale the point's vector by the pixel size
    VectorInMicrons_t vectorInMicrons = m_pixelSize * pointInPixels.getVector();

    // Offset by the top left and the center of a pixel
    SizeInMicrons_t centerOffset = m_pixelSize / SizeInPixels_t(2, 2);
    return m_topLeft + centerOffset + vectorInMicrons;
}

PointInPixels_t
SpacingInfo::convertPointInMicronsToPixels(const PointInMicrons_t & inPoint) const
{
    // Find vector from top left to this input point
    VectorInMicrons_t vectorInMicrons = inPoint - m_topLeft;

    // Divide that vector by the pixel size
    SpatialVector<Ratio> vectorInPixels = vectorInMicrons / m_pixelSize;

    // Clamp and convert to indices
    double xPixels = vectorInPixels.getX().toDouble();
    xPixels = std::max(xPixels, double(0));
    xPixels = std::min(xPixels, double(m_numPixels.getX() - 1));
    xPixels = std::floor(xPixels);

    double yPixels = vectorInPixels.getY().toDouble();
    yPixels = std::max(yPixels, double(0));
    yPixels = std::min(yPixels, double(m_numPixels.getY() - 1));
    yPixels = std::floor(yPixels);

    return PointInPixels_t(static_cast<isize_t>(xPixels), static_cast<isize_t>(yPixels));
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
