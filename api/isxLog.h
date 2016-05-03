#ifndef ISX_LOG_H
#define ISX_LOG_H

#include <iostream>

namespace isx {

/*
 * \return  An output stream for logging messages.
 */
std::ostream& log();

/*
 * \return  An output stream for logging info messages.
 */
std::ostream& info();

/*
 * \return  An output stream for logging warning messages.
 */
std::ostream& warning();

/*
 * \return  An output stream for logging error messages.
 */
std::ostream& error();

class Log
{

public:

    /*!
     * Construct Log to append to standard out.
     */
    Log();

    /*!
     * Destructor.
     */
    ~Log();

    /*!
     * \return  The singleton instance of this class.
     */
    static Log* instance();

    /*!
     * \param   strm    The output stream to which to append.
     */
    void ostream(std::ostream& strm);

    /*!
     * \return  The output stream to which to append.
     */
    std::ostream& ostream() const;

private:
    //! The pointer to the singleton instance.
    static Log* m_instance;

    //! The stream to which to output log messages.
    std::ostream* m_strm;
};

} // namespace isx

#endif
