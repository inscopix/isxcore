#include "isxRatio.h"
#include "isxAssert.h"
#include <iostream>
#include <cmath>
#include <iomanip>

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

/// \param x First integer.
/// \param y Second integer.
/// \return  Whether the multiplication will overflow.
bool isMultiplicationOverflow(int64_t x, int64_t y)
{
    int64_t z = x * y;
    return z != 0 && z / x != y;
}

/// declaration
bool lessThanOrEqualFraction(int64_t first_num, int64_t first_den, int64_t second_num, int64_t second_den);

/// Compare fractions by comparing quotients. If they are the same, compare remainders
bool lessThanFraction(int64_t first_num, int64_t first_den, int64_t second_num, int64_t second_den)
{
    if (second_den == 0)
    {
        return false;
    }
    if (first_den == 0)
    {
        return true;
    }
    if (first_num/first_den < second_num/second_den)
    {
        return true;
    }
    if (first_num/first_den > second_num/second_den)
    {
       return false;
    }
    return !lessThanOrEqualFraction(first_den, first_num%first_den, second_den, second_num%second_num);
}

/// Compare fractions by comparing quotients. If they are the same, compare remainders
bool lessThanOrEqualFraction(int64_t first_num, int64_t first_den, int64_t second_num, int64_t second_den)
{
    if (first_den == 0)
    {
        return true;
    }
    if (second_den == 0)
    {
        return false;
    }
    if (first_num/first_den < second_num/second_den)
    {
        return true;
    }
    if (first_num/first_den > second_num/second_den)
    {
       return false;
    }
    return !lessThanFraction(first_den, first_num%first_den, second_den, second_num%second_num);
}

} // namespace

namespace isx
{

Ratio::Ratio(int64_t inNum, int64_t inDen, bool inSimplify)
{
    ISX_ASSERT(inDen != 0);
    if (inSimplify)
    {
        const int64_t gcd = getGreatestCommonDivisor(inNum, inDen);
        m_num = inNum / gcd;
        m_den = inDen / gcd;
    }
    else
    {
        m_num = inNum;
        m_den = inDen;
    }
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

/*static*/
Ratio
Ratio::fromDouble(double inValue, size_t inPrecision)
{
    int64_t num, den;
    den = int64_t(std::pow(10, inPrecision));
    num = int64_t(std::round(inValue * den));
    return Ratio(num, den);
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
    // There's currently a preference for maintaining the denominator when
    // multiplying by a scalar.
    if (other.m_den == 1)
    {
        return Ratio(m_num * other.m_num, m_den);
    }
    const Ratio thisSim(m_num, m_den, true);
    const Ratio otherSim(other.m_num, other.m_den, true);

    // Detect for the potential overflow and use double to calculate if detected.
    if (isMultiplicationOverflow(thisSim.m_num, otherSim.m_num)
    || isMultiplicationOverflow(thisSim.m_den, otherSim.m_den))
    {
        return Ratio::fromDouble(
                (thisSim.m_num / (double) thisSim.m_den) * (otherSim.m_num / (double) otherSim.m_den));
    }

    return Ratio(thisSim.m_num * otherSim.m_num, thisSim.m_den * otherSim.m_den);
}

Ratio
Ratio::operator /(const Ratio & other) const
{
    if (m_den == other.m_den)
    {
        return Ratio(m_num, other.m_num);
    }
    return (*this * other.getInverse());
}

bool
Ratio::operator ==(const Ratio & other) const
{
    if (m_den == other.m_den)
    {
        return m_num == other.m_num;
    }
    const Ratio thisSim(m_num, m_den, true);
    const Ratio otherSim(other.m_num, other.m_den, true);
    return (thisSim.m_num * otherSim.m_den) == (thisSim.m_den * otherSim.m_num);
}

bool
Ratio::operator !=(const Ratio & other) const
{
    return !(*this == other);
}

bool
Ratio::operator <(const Ratio & other) const
{
    if (m_den == other.m_den)
    {
        return m_num < other.m_num;
    }
    Ratio thisSim(m_num, m_den, true);
    Ratio otherSim(other.m_num, other.m_den, true);

    // If cross multiplication fails, compare as string
    if (isMultiplicationOverflow(thisSim.m_num, otherSim.m_den) ||
        isMultiplicationOverflow(thisSim.m_den, otherSim.m_num))
    {
        return lessThanFraction(thisSim.m_num, thisSim.m_den, otherSim.m_num, otherSim.m_den);
    }

    return (thisSim.m_num * otherSim.m_den) < (thisSim.m_den * otherSim.m_num);
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

Ratio
Ratio::expandWithDenomOf(const Ratio & other) const
{
    return Ratio(m_num * other.m_den, m_den * other.m_den);
}

void
Ratio::serialize(std::ostream& strm) const
{
    strm << m_num << " / " << m_den;
}

} // namespace
