#include "isxSpacingInfo.h"
#include <algorithm>
#include <cmath>

namespace isx
{

SpacingInfo::SpacingInfo() 
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
    m_isValid = true;
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

Rect
SpacingInfo::getFovInPixels() const
{
    const PointInPixels_t topLeft(0, 0);
    isize_t right = m_numPixels.getX();
    if (right > 0)
    {
        right -= 1;
    }
    isize_t bottom = m_numPixels.getY();
    if (bottom > 0)
    {
        bottom -= 1;
    }
    const PointInPixels_t bottomRight(right, bottom);
    return Rect(topLeft, bottomRight);
}

PointInMicrons_t
SpacingInfo::convertPixelsToPointInMicrons(const PointInPixels_t & inPoint) const
{
    Ratio xMicrons = m_topLeft.getX();
    if (m_numPixels.getX() > 0)
    {
        auto xPixels = std::min(inPoint.getX(), int64_t(m_numPixels.getX()) - 1);
        xMicrons = xMicrons + (m_pixelSize.getX() * xPixels);
    }

    Ratio yMicrons = m_topLeft.getY();
    if (m_numPixels.getY() > 0)
    {
        auto yPixels = std::min(inPoint.getY(), int64_t(m_numPixels.getY()) - 1);
        yMicrons = yMicrons + (m_pixelSize.getY() * yPixels);
    }

    return PointInMicrons_t(xMicrons, yMicrons);
}

PointInMicrons_t
SpacingInfo::convertPixelsToMidPointInMicrons(const PointInPixels_t & inPoint) const
{
    PointInMicrons_t topLeft = convertPixelsToPointInMicrons(inPoint);
    return topLeft + VectorInMicrons_t(m_pixelSize.getX() / 2, m_pixelSize.getY() / 2);
}

PointInPixels_t
SpacingInfo::convertMidPointInMicronsToPixels(const PointInMicrons_t & inPoint) const
{
    double xPixels = 0;
    if (m_numPixels.getX() > 0)
    {
        Ratio xMicrons = inPoint.getX() - m_topLeft.getX();
        xPixels = (xMicrons / m_pixelSize.getX()).toDouble();
        xPixels = std::max(xPixels, double(0));
        xPixels = std::min(xPixels, double(m_numPixels.getX() - 1));
        xPixels = std::floor(xPixels);
    }

    double yPixels = 0;
    if (m_numPixels.getY() > 0)
    {
        Ratio xMicrons = inPoint.getY() - m_topLeft.getY();
        yPixels = (xMicrons / m_pixelSize.getY()).toDouble();
        yPixels = std::max(yPixels, double(0));
        yPixels = std::min(yPixels, double(m_numPixels.getY() - 1));
        yPixels = std::floor(yPixels);
    }

    return PointInPixels_t(isize_t(xPixels), isize_t(yPixels));
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


void 
SpacingInfo::setValid(bool inValid)
{
    m_isValid = inValid;
}


bool 
SpacingInfo::isValid() const
{
    return m_isValid;
}

SpacingInfo 
SpacingInfo::getDefault()
{
    SpacingInfo defaultSpacingInfo(SizeInPixels_t(1440, 1080), SizeInMicrons_t(DEFAULT_PIXEL_SIZE, DEFAULT_PIXEL_SIZE), PointInMicrons_t(0, 0));
    return defaultSpacingInfo;
}

SpacingInfo 
SpacingInfo::getDefault(const SizeInPixels_t & numPixels)
{
    SpacingInfo defaultSpacingInfo(numPixels, SizeInMicrons_t(DEFAULT_PIXEL_SIZE, DEFAULT_PIXEL_SIZE), PointInMicrons_t(0, 0));
    return defaultSpacingInfo;
}

} // namespace
