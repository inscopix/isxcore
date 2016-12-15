#include "isxObject.h"
#include "isxTimingInfo.h"
#include "isxSpacingInfo.h"
#include <memory>
#include <string>
#include <vector>


namespace isx {

    /// A class for nvista recording XML
    /// This is considered immutable - we will never write to it
    ///
    class RecordingXml : public Object
    {
    public:
        /// Construct recording from a given file.
        ///
        /// \param inPath Path to recording xml file.
        /// \throw isx::ExceptionFileIO     If the file cannot be read.
        RecordingXml(const std::string & inPath);

        /// Destructor.
        ///
        ~RecordingXml();
        
        /// \return the filenames of hdf5 files included in the XML
        ///
        const std::vector<std::string> & getFileNames();
        
        /// \return the timing Info
        ///
        TimingInfo getTimingInfo();

        /// \return the vector of dropped frame numbers
        ///
        const std::vector<isize_t> & getDroppedFrames() const;
        
        /// \return the spacing info
        ///
        SpacingInfo getSpacingInfo();
        
        /// serialize to stream
        ///
        void serialize(std::ostream& strm) const override;
        

    private:
        class Impl;
        std::unique_ptr<Impl> m_pImpl;   
    };
}
