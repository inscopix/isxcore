#ifndef ISX_CELL_SET_SIMPLE_H
#define ISX_CELL_SET_SIMPLE_H

#include "isxCellSet.h"
#include "isxCoreFwd.h"
#include "isxAsync.h"
#include "isxMutex.h"

#include <fstream>
#include <map>
#include <memory>

namespace isx
{

/// Forward declare needed because CellSetFile is in src.
class CellSetFile;
class IoTaskTracker;
/// A class for a containing cells extracted from a movie
///
/// Currently all reads/writes happen on calling threads and
/// not on the IO thread.
class CellSetSimple 
    : public CellSet
    , public std::enable_shared_from_this<CellSetSimple>
{
public:

    /// Empty constructor.
    ///
    /// This creates an invalid cell set and is for allocation purposes only.
    CellSetSimple();

    /// Read constructor.
    ///
    /// This opens an existing cell set file and reads information from its
    /// header.
    ///
    /// \param  inFileName  The name of the cell set file to read.
    /// \throw  isx::ExceptionFileIO    If reading the cell set file fails.
    /// \throw  isx::ExceptionDataIO    If parsing the cell set file fails.
    CellSetSimple(const std::string & inFileName);

    /// Write constructor.
    ///
    /// This opens a new file, writes header information.
    ///
    /// \param  inFileName      The name of the cell set file to write.
    /// \param  inTimingInfo    The timing information of the cell set.
    /// \param  inSpacingInfo   The spacing information of the cell set.
    /// \throw  isx::ExceptionFileIO    If writing the cell set file fails.
    /// \throw  isx::ExceptionDataIO    If formatting the cell set data fails.
    CellSetSimple(const std::string & inFileName,
            const TimingInfo & inTimingInfo,
            const SpacingInfo & inSpacingInfo);

    /// Destructor.
    ///
    ~CellSetSimple();

    // Overrides
    bool 
    isValid() const override;

    std::string 
    getFileName() const override;

    const isize_t 
    getNumCells() override;

    isx::TimingInfo 
    getTimingInfo() const override;

    isx::TimingInfos_t 
    getTimingInfosForSeries() const override;

    isx::SpacingInfo 
    getSpacingInfo() const override;

    SpFTrace_t 
    getTrace(isize_t inIndex) override;

    void 
    getTraceAsync(isize_t inIndex, CellSetGetTraceCB_t inCallback) override;

    SpImage_t 
    getImage(isize_t inIndex) override;

    void 
    getImageAsync(isize_t inIndex, CellSetGetImageCB_t inCallback) override;  

    void 
    writeImageAndTrace(
            isize_t inIndex,
            SpImage_t & inImage,
            SpFTrace_t & inTrace,
            const std::string & inName = std::string()) override; 

    bool 
    isCellValid(isize_t inIndex) override;

    void 
    setCellValid(isize_t inIndex, bool inIsValid) override;

    std::string 
    getCellName(isize_t inIndex) override;

    void 
    setCellName(isize_t inIndex, const std::string & inName) override;

    void 
    cancelPendingReads() override;

private:

    /// True if the cell set is valid, false otherwise.
    bool m_valid = false;

    /// The cell set file to read/write from/to.
    std::shared_ptr<CellSetFile> m_file;
    std::shared_ptr<IoTaskTracker>      m_ioTaskTracker;
};

}
#endif // ISX_CELL_SET_SIMPLE_H
