#include "isxSpacingInfo.h"

namespace isx
{

SpacingInfo::SpacingInfo()
    : m_numPixels(Point<isize_t>(1440, 1080))
    , m_pixelSize(Point<SpacingInfo::Coord_t>(Ratio(22, 10), Ratio(22, 10)))
    , m_topLeft(Point<SpacingInfo::Coord_t>(0, 0))
{
}

SpacingInfo::SpacingInfo(
    const Point<isize_t>& numPixels,
    const Point<SpacingInfo::Coord_t>& pixelSize,
    const Point<SpacingInfo::Coord_t>& topLeft)
    : m_numPixels(numPixels)
    , m_pixelSize(pixelSize)
    , m_topLeft(topLeft)
{
}

const Point<SpacingInfo::Coord_t>&
SpacingInfo::getTopLeft() const
{
    return m_topLeft;
}

Point<SpacingInfo::Coord_t>
SpacingInfo::getBottomRight() const
{
    return m_topLeft + getTotalSize();
}

const Point<SpacingInfo::Coord_t>&
SpacingInfo::getPixelSize() const
{
    return m_pixelSize;
}

const Point<isize_t>&
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

Point<SpacingInfo::Coord_t>
SpacingInfo::getTotalSize() const
{
    return m_pixelSize * m_numPixels;
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
