#ifndef ISX_RATIO_H
#define ISX_RATIO_H

#include <cstdint>
#include "isxObject.h"

namespace isx
{

/// A rational number with unsigned 64-bit numators and deninators.
///
class Ratio : public isx::Object
{

public:

    /// Default constructor.
    ///
    /// \param  num     The numator of the rational number.
    /// \param  den     The deninator of the rational number.
    Ratio(int64_t num = 0, int64_t den = 1);

    /// Return the numerator.
    ///
    /// \return         The numerator.
    int64_t getNum() const;

    /// Return the denominator.
    ///
    /// \return         The denominator.
    int64_t getDen() const;

    /// Return the floating point evaluation of this.
    ///
    /// \return         The floating point evaluation of this.
    double toDouble() const;

    /// Return the result of adding another rational number to this.
    ///
    /// \param   other  The other rational number to add to this.
    /// \return         The sum of this and the other rational number.
    isx::Ratio operator +(const isx::Ratio& other) const;

    /// Returns the result of subtracting another rational number from this.
    ///
    /// \param   other  The other rational number to subtract.
    /// \return         The difference between this and the other rational number.
    isx::Ratio operator -(const isx::Ratio& other) const;

    /// Returns the result of multiplying this by another rational number.
    ///
    /// \param   other  The other rational number with which to multiply.
    /// \return         The product of the other rational number and this.
    isx::Ratio operator *(const isx::Ratio& other) const;

    /// Returns the result of dividing this by another rational number.
    ///
    /// \param   other  The other rational number by which to divide.
    /// \return         The result of diving this by the other rational number.
    isx::Ratio operator /(const isx::Ratio& other) const;

    /// Compares this with another rational number exactly.
    ///
    /// \param   other  The rational number with which to compare.
    /// \return         True if this is exactly equal to other, false otherwise.
    bool operator ==(const isx::Ratio& other) const;

    /// Compares this with another rational number exactly.
    ///
    /// \param   other  The rational number with which to compare.
    /// \return         True if this is not exactly equal to other, false otherwise.
    bool operator !=(const isx::Ratio& other) const;

    /// Returns true if this is less than another rational number.
    ///
    /// \param   other  The rational number with which to compare.
    /// \return         True, if this is less than the other rational number.
    bool operator <(const isx::Ratio& other) const;

    /// Returns true if this is less than or equal to another rational number.
    ///
    /// \param   other  The rational number with which to compare.
    /// \return         True, if this is less than or equal to the other rational number.
    bool operator <=(const isx::Ratio& other) const;

    /// Returns true if this is greater than another rational number.
    ///
    /// \param   other  The rational number with which to compare.
    /// \return         True, if this is greater than the other rational number.
    bool operator >(const isx::Ratio& other) const;

    /// Returns true if this is greater than or equal to another rational number.
    ///
    /// \param   other  The rational number with which to compare.
    /// \return         True, if this is greater than or equal to the other rational number.
    bool operator >=(const isx::Ratio& other) const;

    // Overrides
    virtual void serialize(std::ostream& strm) const;

private:

    /// The numator of the fraction storing the number of seconds since the Unix epoch.
    int64_t m_num;

    /// The deninator of the fraction storing the number of seconds since the Unix epoch.
    int64_t m_den;

}; // class

} // namespace

#endif // ISX_RATIO_H
