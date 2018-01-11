#ifndef ISX_RATIO_H
#define ISX_RATIO_H

#include <cstdint>
#include "isxObject.h"

namespace isx
{

/// A rational number with unsigned 64-bit numerators and denominators.
///
/// The constructor defaults to creating 0/1 and thus also allows you to
/// only specify one integer to represent integral rational numbers.
class Ratio : public Object
{

public:

    /// Fully specified constructor.
    ///
    /// \param  inNum       The numerator of the rational number.
    /// \param  inDen       The denominator of the rational number.
    /// \param  inSimplify  If true, simplify the rational number by finding the
    ///                     greatest common divisor of the numerator and denominator.
    ///                     Otherwise, store them as given.
    Ratio(int64_t inNum = 0, int64_t inDen = 1, bool inSimplify = false);

    /// Return the numerator.
    ///
    /// \return         The numerator.
    int64_t getNum() const;

    /// Return the denominator.
    ///
    /// \return         The denominator.
    int64_t getDen() const;

    /// \return     The inverse of this.
    ///
    Ratio getInverse() const;

    /// Return the floating point evaluation of this.
    ///
    /// \return         The floating point evaluation of this.
    double toDouble() const;

    /// Return a Ratio representing the floating point value as close as possible
    ///
    /// \param inValue the value to be converted to a ratio
    /// \param inPrecision the number of digits after the decimal point to take into account
    static Ratio fromDouble(double inValue, size_t inPrecision = 3);

    /// Return the result of adding another rational number to this.
    ///
    /// \param   other  The other rational number to add to this.
    /// \return         The sum of this and the other rational number.
    Ratio operator +(const Ratio & other) const;

    /// Add another rational number to this in-place.
    ///
    /// \param   other  The other rational number to add to this.
    /// \return         The sum of this and the other rational number.
    Ratio & operator +=(const Ratio & other);

    /// Return the result of subtracting another rational number from this.
    ///
    /// \param   other  The other rational number to subtract.
    /// \return         The difference between this and the other rational number.
    Ratio operator -(const Ratio & other) const;

    /// Subtract another rational number from this in-place.
    ///
    /// \param   other  The other rational number to subtract.
    /// \return         The difference between this and the other rational number.
    Ratio & operator -=(const Ratio & other);

    /// Returns the result of multiplying this by another rational number.
    ///
    /// \param   other  The other rational number with which to multiply.
    /// \return         The product of the other rational number and this.
    Ratio operator *(const Ratio & other) const;

    /// Returns the result of dividing this by another rational number.
    ///
    /// \param   other  The other rational number by which to divide.
    /// \return         The result of diving this by the other rational number.
    Ratio operator /(const Ratio & other) const;

    /// Compares this with another rational number exactly.
    ///
    /// \param   other  The rational number with which to compare.
    /// \return         True if this is exactly equal to other, false otherwise.
    bool operator ==(const Ratio & other) const;

    /// Compares this with another rational number exactly.
    ///
    /// \param   other  The rational number with which to compare.
    /// \return         True if this is not exactly equal to other, false otherwise.
    bool operator !=(const Ratio & other) const;

    /// Returns true if this is less than another rational number.
    ///
    /// \param   other  The rational number with which to compare.
    /// \return         True, if this is less than the other rational number.
    bool operator <(const Ratio & other) const;

    /// Returns true if this is less than or equal to another rational number.
    ///
    /// \param   other  The rational number with which to compare.
    /// \return         True, if this is less than or equal to the other rational number.
    bool operator <=(const Ratio & other) const;

    /// Returns true if this is greater than another rational number.
    ///
    /// \param   other  The rational number with which to compare.
    /// \return         True, if this is greater than the other rational number.
    bool operator >(const Ratio & other) const;

    /// Returns true if this is greater than or equal to another rational number.
    ///
    /// \param   other  The rational number with which to compare.
    /// \return         True, if this is greater than or equal to the other rational number.
    bool operator >=(const Ratio & other) const;

    /// \return the largest ratio with other's denom that is not greater than
    ///         the this ratio in value
    /// \param other The rational number for which to return the above
    ///
    Ratio floorToDenomOf(const Ratio & other) const;

    /// \return this Ratio expanded with denominator of other Ratio.
    /// \param other Ratio from which to use the denominator for expansion.
    ///
    Ratio expandWithDenomOf(const Ratio & other) const;

    // Overrides
    void serialize(std::ostream & strm) const override;

private:

    /// The numerator of the fraction storing the number of seconds since the Unix epoch.
    int64_t m_num;

    /// The denominator of the fraction storing the number of seconds since the Unix epoch.
    int64_t m_den;

}; // class

} // namespace

#endif // ISX_RATIO_H
