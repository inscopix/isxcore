#include "isxRect.h"

namespace isx
{

isize_t
Rect::width() const
{
    return m_bottomRight.getX() - m_topLeft.getX() + 1;
}

isize_t
Rect::height() const
{
    return m_bottomRight.getY() - m_topLeft.getY() + 1;
}

isize_t
Rect::x() const
{
    return m_topLeft.getX();
}

isize_t
Rect::y() const
{
    return m_topLeft.getY();
}

bool
Rect::operator ==(const Rect & inOther) const
{
    return (m_topLeft == inOther.m_topLeft)
        && (m_bottomRight == inOther.m_bottomRight);
}

} // namespace
