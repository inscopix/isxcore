#include "isxRatio.h"
#include "isxAssert.h"
#include <iostream>
#include <cmath>

namespace
{

/// Get the greatest common divisor/factor of two integers.
///
/// This uses the modulo version of Euclid's algorithm.
///
/// \param x First integer.
/// \param y Second integer.
/// \return  The greatest common divisor of x and y.
int64_t getGreatestCommonDivisor(int64_t x, int64_t y)
{
    int64_t z;
    while (y != 0)
    {
        z = y;
        y = x % y;
        x = z;
    }
    return x;
}

/// \param x First integer.
/// \param y Second integer.
/// \return  The least common multiple of x and y.
int64_t getLeastCommonMultiple(int64_t x, int64_t y)
{
    const int64_t gcd = ::getGreatestCommonDivisor(x, y);
    return x * (y / gcd);
}

} // namespace

namespace isx
{

Ratio::Ratio(int64_t num, int64_t den)
: m_num(num)
, m_den(den)
{
    ISX_ASSERT(den != 0);
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

Ratio
Ratio::getInverse() const
{
    return Ratio(m_den, m_num);
}

double
Ratio::toDouble() const
{
    return double(m_num) / double(m_den);
}

Ratio
Ratio::operator +(const Ratio & other) const
{
    if (m_den == other.m_den)
    {
        return Ratio(m_num + other.m_num, m_den);
    }
    const int64_t den = ::getLeastCommonMultiple(m_den, other.m_den);
    const int64_t num = (m_num * (den / m_den)) + (other.m_num * (den / other.m_den));
    return Ratio(num, den);
}

Ratio &
Ratio::operator +=(const Ratio & other)
{
    *this = *this + other;
    return *this;
}

Ratio
Ratio::operator -(const Ratio & other) const
{
    if (m_den == other.m_den)
    {
        return Ratio(m_num - other.m_num, m_den);
    }
    const int64_t den = ::getLeastCommonMultiple(m_den, other.m_den);
    const int64_t num = (m_num * (den / m_den)) - (other.m_num * (den / other.m_den));
    return Ratio(num, den);
}

Ratio &
Ratio::operator -=(const Ratio & other)
{
    *this = *this - other;
    return *this;
}

Ratio
Ratio::operator *(const Ratio & other) const
{
    int64_t num = m_num * other.m_num;
    int64_t den = m_den * other.m_den;
    return Ratio(num, den);
}

Ratio
Ratio::operator /(const Ratio & other) const
{
    int64_t num = m_num * other.m_den;
    int64_t den = m_den * other.m_num;
    return Ratio(num, den);
}

bool
Ratio::operator ==(const Ratio & other) const
{
    return (m_num * other.m_den) == (m_den * other.m_num);
}

bool
Ratio::operator !=(const Ratio & other) const
{
    return !(*this == other);
}

bool
Ratio::operator <(const Ratio & other) const
{
    return (m_num * other.m_den) < (m_den * other.m_num);
}

bool
Ratio::operator <=(const Ratio & other) const
{
    return (*this < other) || (*this == other);
}

bool
Ratio::operator >(const Ratio & other) const
{
    return other < *this;
}

bool
Ratio::operator >=(const Ratio & other) const
{
    return (*this > other) || (*this == other);
}

Ratio
Ratio::floorToDenomOf(const Ratio & other) const
{
    double tv = toDouble();
    int64_t od = other.getDen();
    return Ratio(int64_t(std::floor(tv * double(od))), od);
}

void
Ratio::serialize(std::ostream& strm) const
{
    strm << m_num << " / " << m_den;
}

} // namespace
