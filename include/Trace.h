
namespace mosaic {

/*!
 * A function of time with a discrete domain.
 */
template <class T>
class Trace {

public:

    /*!
     * Trace constructor.
     */
    Trace();

    /*!
     * Time destructor.
     */
    ~Trace();

    /*!
     * Read access to a range value by index.
     *
     * \param   idx     The index.
     * \return          The ith range value.
     */
    const T operator[] (std::size_t idx);

    /*!
     * Write access to a range value by index.
     *
     * \param   idx     The domain index.
     * \return          The new ith range value.
     */
    T operator[] (std::size_t idx);

private:

    //! The discrete domain of the function.
    TimeGrid domain;

    //! The discrete range of the function.
    T * range;

} // class

} // namespace

