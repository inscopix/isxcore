#include "isxRecording.h"
#include "isxRecording_internal.h"
#include "isxLog.h"
#include "H5Cpp.h"

#include <iostream>

namespace isx {
Recording::Impl::~Impl(){};
Recording::Impl::Impl(){};
Recording::Impl::Impl(const std::string & inPath)
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

bool
Recording::Impl::isValid() const
{
    return m_isValid;
}

Recording::Impl::tH5File_SP
Recording::Impl::getH5FileRef()
{
    return m_file;
}

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

} // namespace isx

