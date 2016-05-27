#include "isxRecording.h"
#include "isxMovie.h"
#include "isxLog.h"

#include "H5Cpp.h"

#include <iostream>

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
        try
        {
            // Turn off the auto-printing when failure occurs so that we can
            // handle the errors appropriately
            H5::Exception::dontPrint();
            
            // Open an existing file and dataset.
            m_file.reset(new H5::H5File(m_path.c_str(), H5F_ACC_RDONLY));

            // no exception until here --> this is a valid file
            m_isValid = true;
        }  // end of try block
        
        // catch failure caused by the H5File operations
        catch(H5::FileIException error)
        {
            error.printError();
        }
        
        // catch failure caused by the DataSet operations
        catch(H5::DataSetIException error)
        {
            error.printError();
        }
        
        catch(...)
        {
            ISX_LOG_ERROR("Unhandled exception.");
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

private:
    bool m_isValid = false;
    std::string m_path;
    
    tH5File_SP m_file;
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

tMovie_SP
Recording::getMovie(const std::string & inDataSetName)
{
    return tMovie_SP();
}

} // namespace isx

