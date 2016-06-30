#include "isxSpacingInfo.h"

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
SpacingInfo::convertPointInPixelsToMicrons(const PointInPixels_t inPoint) const
{
    // Clamp pixel indices that exceed range and protect against subtraction from 0
    isize_t xPixels = inPoint.getX();
    isize_t xNumPixels = m_numPixels.getX();
    if (xNumPixels > 0 && xPixels >= xNumPixels)
    {
        xPixels = xNumPixels - 1;
    }

    isize_t yPixels = inPoint.getY();
    isize_t yNumPixels = m_numPixels.getY();
    if (yNumPixels > 0 && yPixels >= yNumPixels)
    {
        yPixels = yNumPixels - 1;
    }

    PointInPixels_t pointInPixels(xPixels, yPixels);

    // Multiply by pixel size and offset by center
    PointInMicrons_t offset = m_pixelSize * PointInMicrons_t(Ratio(1, 2), Ratio(1, 2));
    return (m_pixelSize * pointInPixels) + offset;
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
