#ifndef ISX_POINT_H
#define ISX_POINT_H

#include <string>

namespace isx {

/*!
 * A pair of 2D (x, y) coordinates.
 */
class Point {

public:

    /*!
     * Default constructor.
     *
     * Initially the coordinates are (0, 0).
     */
    Point();

    /*!
     * Constructor that allows for specification of (x, y) coordinates.
     *
     * \param   x       The x coordinate.
     * \param   y       The y coordinate.
     */
    Point(double x, double y);

    /*!
     * Returns a new point which is the result of this plus another point.
     *
     * \param   other   The point to add.
     * \return  The result of adding other to this.
     */
    isx::Point plus(const isx::Point& other) const;

    /*!
     * Returns a new point which is the result of this plus a scalar.
     *
     * \param   scalar  The scalar to add.
     * \return  The result of adding scalar to this.
     */
    isx::Point plus(double scalar) const;

    /*!
     * Returns a new point which is the result of this minus another point.
     *
     * \param   other   The point to subtract.
     * \return  The result of subtracting other from this.
     */
    isx::Point minus(const isx::Point& other) const;

    /*!
     * Returns a new point which is the result of this plus a scalar.
     *
     * \param   scalar  The scalar to subtract.
     * \return  The result of subtracting scalar from this.
     */
    isx::Point minus(double scalar) const;

    /*!
     * Returns a new point which is the result of this times another point.
     *
     * \param   other   The point with which to multiply.
     * \return  The result of multiplying this with other.
     */
    isx::Point times(const isx::Point& other) const;

    /*!
     * Returns a new point which is the result of this times a scalar.
     *
     * \param   scalar  The scalar with which to multiply.
     * \return  The result of multiplying this with scalar.
     */
    isx::Point times(double scalar) const;

    /*!
     * Returns a new point which is the result of this divided by another point.
     *
     * \param   other   The point with which to divide.
     * \return  The result of dividing this with other.
     */
    isx::Point divide(const isx::Point& other) const;

    /*!
     * Returns a new point which is the result of this divided by a scalar.
     *
     * \param   scalar  The scalar with which to divide.
     * \return  The result of dividing this with scalar.
     */
    isx::Point divide(double scalar) const;

    /*!
     * Returns a string representation of the point as "(x, y)".
     *
     * \param   prec    The number of decimal places.
     * \return  A string representation of the point as "(x, y)".
     */
    std::string toString(int prec = 2) const;

private:

    //! The x coordinate.
    double m_x;

    //! The y coordinate.
    double m_y;

}; // class

} // namespace

#endif // ISX_POINT_H
