#ifndef ISX_CELL_SET_SERIES_H
#define ISX_CELL_SET_SERIES_H

#include "isxCellSet.h"
#include "isxCoreFwd.h"
#include "isxAsync.h"
#include "isxMutex.h"

#include <fstream>
#include <map>
#include <memory>

namespace isx
{

/// A class for a containing cells extracted from a movie series
///
class CellSetSeries 
    : public CellSet
    , public std::enable_shared_from_this<CellSetSeries>
{
public:

    /// Empty constructor.
    ///
    /// This creates an invalid cell set and is for allocation purposes only.
    CellSetSeries();

    /// Read constructor.
    ///
    /// This opens an existing cell set file and reads information from its
    /// header.
    ///
    /// \param  inFileNames  The name of the cell set file to read.
    /// \param  enableWrite     Set to true to open in read-write mode
    /// \throw  isx::ExceptionFileIO    If reading the cell set file fails.
    /// \throw  isx::ExceptionDataIO    If parsing the cell set file fails.
    CellSetSeries(const std::vector<std::string> & inFileNames, bool enableWrite = false);

    /// Destructor.
    ///
    ~CellSetSeries();

    // Overrides
    bool 
    isValid() const override;

    void
    closeForWriting() override;

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


    CellSet::CellStatus 
    getCellStatus(isize_t inIndex) override;

    void 
    setCellStatus(isize_t inIndex, CellSet::CellStatus inStatus) override;

    std::string
    getCellStatusString(isize_t inIndex) override;

    std::string 
    getCellName(isize_t inIndex) override;

    void 
    setCellName(isize_t inIndex, const std::string & inName) override;

    void 
    cancelPendingReads() override;

    bool
    isRoiSet() const override;

private:


    TimingInfo getGaplessTimingInfo();

    /// True if the cell set is valid, false otherwise.
    bool m_valid = false;
    
    TimingInfo                          m_timingInfo;       // Global timing info
    std::vector<SpCellSet_t>            m_cellSets;
    

};

}
#endif // ISX_CELL_SET_SIMPLE_H
