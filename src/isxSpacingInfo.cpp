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

PointInMicrons_t
SpacingInfo::convertPointInPixelsToMicrons(const PointInPixels_t & inPoint) const
{
    Ratio xMicrons = m_topLeft.getX();
    if (m_numPixels.getX() > 0)
    {
        isize_t xPixels = std::min(inPoint.getX(), m_numPixels.getX() - 1);
        xMicrons = xMicrons + (m_pixelSize.getX() / 2);

        // Offset by half pixel size to get to center.
        xMicrons = xMicrons + (m_pixelSize.getX() * xPixels);
    }

    Ratio yMicrons = m_topLeft.getY();
    if (m_numPixels.getY() > 0)
    {
        isize_t yPixels = std::min(inPoint.getY(), m_numPixels.getY() - 1);
        yMicrons = yMicrons + (m_pixelSize.getY() * yPixels);

        // Offset by half pixel size to get to center.
        yMicrons = yMicrons + (m_pixelSize.getY() / 2);
    }

    return PointInMicrons_t(xMicrons, yMicrons);
}

PointInPixels_t
SpacingInfo::convertPointInMicronsToPixels(const PointInMicrons_t & inPoint) const
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

} // namespace
