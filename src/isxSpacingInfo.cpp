#include "isxSpacingInfo.h"

namespace isx
{

SpacingInfo::SpacingInfo()
    : m_topLeft(Point<SpacingInfo::Coord_t>(0, 0))
    , m_pixelSize(Point<SpacingInfo::Coord_t>(isx::Ratio(22, 10), isx::Ratio(22, 10)))
    , m_numPixels(Point<size_t>(1440, 1080))
{
}

SpacingInfo::SpacingInfo(
    const Point<SpacingInfo::Coord_t>& topLeft,
    const Point<SpacingInfo::Coord_t>& pixelSize,
    const Point<size_t>& numPixels)
    : m_topLeft(topLeft)
    , m_pixelSize(pixelSize)
    , m_numPixels(numPixels)
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

const Point<size_t>&
SpacingInfo::getNumPixels() const
{
    return m_numPixels;
}

Point<SpacingInfo::Coord_t>
SpacingInfo::getTotalSize() const
{
    return m_pixelSize * m_numPixels;
}

void
SpacingInfo::serialize(std::ostream& strm) const
{
    strm << "SpacingInfo("
            << "TopLeft=" << m_topLeft
            << "PixelSize=" << m_pixelSize
            << "NumPixels=" << m_numPixels
         << ")";
}

} // namespace
