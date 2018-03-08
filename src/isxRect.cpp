#include "isxRect.h"
#include "isxAssert.h"

namespace isx
{

isize_t
Rect::width() const
{
    ISX_ASSERT(m_bottomRight.getX() >= m_topLeft.getX());
    return isize_t(m_bottomRight.getX() - m_topLeft.getX() + 1);
}

isize_t
Rect::height() const
{
    ISX_ASSERT(m_bottomRight.getY() >= m_topLeft.getY());
    return isize_t(m_bottomRight.getY() - m_topLeft.getY() + 1);
}

int64_t
Rect::x() const
{
    return m_topLeft.getX();
}

int64_t
Rect::y() const
{
    return m_topLeft.getY();
}

bool
Rect::isValid() const
{
    return !((m_topLeft == SpatialPoint<int64_t>(-1, -1))
        && (m_bottomRight == SpatialPoint<int64_t>(-1, -1)));
}

bool
Rect::operator ==(const Rect & inOther) const
{
    return (m_topLeft == inOther.m_topLeft)
        && (m_bottomRight == inOther.m_bottomRight);
}

::std::ostream&
operator<<(::std::ostream & inStream, const isx::Rect & inRect)
{
    inStream << "Rect(topLeft = (" << inRect.x() << ", " << inRect.y() << "), size = " << inRect.width() << "x" << inRect.height() << ")";
    return inStream;
}

} // namespace
