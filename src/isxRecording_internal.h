#include "isxRecording.h"
#include "H5Cpp.h"

#include <memory>

namespace isx {
///
/// Internal implementation class for recording
///
class Recording::Impl
{
public:
    /// shared_ptr type
    ///
    typedef std::shared_ptr<H5::H5File> tH5File_SP;
    
    /// Constructor
    ///
    Impl();

    /// Constructor for Recording from file
    /// \param inPath to file on disk
    ///
    Impl(const std::string & inPath);

    /// Destructor
    ///
    ~Impl();

    /// \return whether this is a valid internal recording object.
    ///
    bool
    isValid() const;

    /// Accessor to H5File object (ref-counted)
    /// \return a shared pointer to the H5File object for this recording
    ///
    tH5File_SP
    getH5FileRef();

private:
    bool m_isValid = false;
    std::string m_path;
    
    tH5File_SP m_file;
};

} // namespace isx
