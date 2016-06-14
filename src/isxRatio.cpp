#include "isxRatio.h"

#include <iostream>
#include <cmath>

namespace isx
{

Ratio::Ratio(int64_t num, int64_t den)
: m_num(num)
, m_den(den)
{
}

int64_t
Ratio::getNum() const
{
    return m_num;
}

int64_t
Ratio::getDen() const
{
    return m_den;
}

double
Ratio::toDouble() const
{
    return double(m_num) / double(m_den);
}

isx::Ratio
Ratio::operator +(const isx::Ratio& other) const
{
    int64_t num, den;
    if (m_den == other.m_den)
    {
        num = m_num + other.m_num;
        den = m_den;
    }
    else
    {
        num = (m_num * other.m_den) + (m_den * other.m_num);
        den = m_den * other.m_den;
    }
    return isx::Ratio(num, den);
}

isx::Ratio
Ratio::operator -(const isx::Ratio& other) const
{
    int64_t num, den;
    if (m_den == other.m_den)
    {
        num = m_num - other.m_num;
        den = m_den;
    }
    else
    {
        num = (m_num * other.m_den) - (m_den * other.m_num);
        den = m_den * other.m_den;
    }
    return isx::Ratio(num, den);
}

isx::Ratio
Ratio::operator *(const isx::Ratio& other) const
{
    int64_t num = m_num * other.m_num;
    int64_t den = m_den * other.m_den;
    return isx::Ratio(num, den);
}

isx::Ratio
Ratio::operator /(const isx::Ratio& other) const
{
    int64_t num = m_num * other.m_den;
    int64_t den = m_den * other.m_num;
    return isx::Ratio(num, den);
}

bool
Ratio::operator ==(const isx::Ratio& other) const
{
    return (m_num * other.m_den) == (m_den * other.m_num);
}

bool
Ratio::operator !=(const isx::Ratio& other) const
{
    return !(*this == other);
}

bool
Ratio::operator <(const isx::Ratio& other) const
{
    return (m_num * other.m_den) < (m_den * other.m_num);
}

bool
Ratio::operator <=(const isx::Ratio& other) const
{
    return (*this < other) || (*this == other);
}

bool
Ratio::operator >(const isx::Ratio& other) const
{
    return other < *this;
}

bool
Ratio::operator >=(const isx::Ratio& other) const
{
    return (*this > other) || (*this == other);
}
    
Ratio
Ratio::floorWithThisDenom(const isx::Ratio& other) const
{
    double ov = double(other.getNum()) / double (other.getDen());
    return Ratio(int64_t(std::floor(ov * double(m_den))), m_den);
}

void
Ratio::serialize(std::ostream& strm) const
{
    strm << m_num << " / " << m_den;
}

} // namespace
