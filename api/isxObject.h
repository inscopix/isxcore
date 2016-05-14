#ifndef ISX_OBJECT_H
#define ISX_OBJECT_H

#include <iostream>

namespace isx {

/*!
 * The base class of all Inscopix objects.
 */
class Object {

public:

    /*!
     * Serialize the object into an output stream.
     *
     * \param   strm    The output stream.
     */
    virtual void serialize(std::ostream& strm) const = 0;

    /*!
     * Conversion to a string.
     */
    std::string toString() const;

    /*!
     * Conversion to a string with specific floating point precision.
     */
    std::string toString(uint8_t prec) const;

}; // class

/*!
 * One time overload of ostream << operator.
 *
 * \param   strm    The output stream to which to print.
 * \param   object  The object to print.
 */
::std::ostream& operator<<(::std::ostream& strm, const isx::Object& object);

} // namespace

#endif // ISX_OBJECT_H
