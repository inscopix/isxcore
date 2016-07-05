#include "isxRecording.h"
#include "isxNVistaMovie.h"
#include "isxException.h"
#include "isxAssert.h"
#include "isxHdf5FileHandle.h"


#include <iostream>
#include <fstream>

namespace isx {

class Recording::Impl
{
public:
    /// Constructor
    ///
    Impl(){}

    /// Constructor for Recording from file
    /// \param inPath to file on disk
    ///
    Impl(const std::string & inPath)
    : m_path(inPath)
    {
        if (exists())
        {
            try
            {
                // Turn off the auto-printing when failure occurs so that we can
                // handle the errors appropriately
                H5::Exception::dontPrint();

                m_file = std::make_shared<H5::H5File>(m_path.c_str(), H5F_ACC_RDONLY);
                m_fileHandle = std::make_shared<Hdf5FileHandle>(m_file, H5F_ACC_RDONLY);
                m_movie = std::make_shared<NVistaMovie>(m_fileHandle, "/images");

                // no exception until here --> this is a valid file
                m_isValid = true;
            }  // end of try block

            catch (const H5::FileIException& error)
            {
                ISX_THROW(isx::ExceptionFileIO,
                    "Failure caused by H5 File operations.\n", error.getDetailMsg());
            }

            catch (const H5::DataSetIException& error)
            {
                ISX_THROW(isx::ExceptionDataIO,
                    "Failure caused by H5 Dataset operations.\n", error.getDetailMsg());
            }

            catch (...)
            {
                ISX_ASSERT(false, "Unhandled exception.");
            }
        }
    }

    /// Destructor
    ///
    ~Impl(){}

    /// \return whether this is a valid internal recording object.
    ///
    bool
    isValid() const
    {
        return m_isValid;
    }
    
    /// \return Hdf5FileHandle
    ///
    SpHdf5FileHandle_t
    getHdf5FileHandle() const
    {
        return m_fileHandle;
    }

    void
    serialize(std::ostream& strm) const
    {
        strm << m_path;
    }

    SpMovie_t 
    getMovie()
    {
        return m_movie;
    }

    std::string 
    getName()
    {
        return m_path;
    }

private:

    bool exists()
    {
        std::ifstream infile(m_path.c_str());
        return infile.good();
    }

    bool m_isValid = false;
    std::string m_path;
    
    SpH5File_t m_file;
    SpHdf5FileHandle_t m_fileHandle;
    SpMovie_t  m_movie;
};

Recording::Recording()
{
    m_pImpl.reset(new Impl());
}

Recording::Recording(const std::string & inPath)
{
    m_pImpl.reset(new Impl(inPath));
}

Recording::~Recording()
{
}

bool
Recording::isValid() const
{
    return m_pImpl->isValid();
}

SpHdf5FileHandle_t
Recording::getHdf5FileHandle()
{
    return m_pImpl->getHdf5FileHandle();
}

void
Recording::serialize(std::ostream& strm) const
{
    m_pImpl->serialize(strm);
}

SpMovie_t 
Recording::getMovie()
{
    return m_pImpl->getMovie();
}


std::string 
Recording::getName()
{
    return m_pImpl->getName();
}

} // namespace isx