#ifndef ISX_CELL_SET_H
#define ISX_CELL_SET_H

#include "isxTimingInfo.h"
#include "isxSpacingInfo.h"
#include "isxImage.h"
#include "isxTrace.h"
#include "isxCoreFwd.h"

#include <fstream>

namespace isx
{

/// Forward declare needed because CellSetFile is in src.
class CellSetFile;

/// A class for a containing cells extracted from a movie
///
/// Currently all reads/writes happen on calling threads and
/// not on the IO thread.
class CellSet
{

public:

    /// Read constructor.
    ///
    /// This opens an existing cell set file and reads information from its
    /// header.
    ///
    /// \param  inFileName  The name of the cell set file to read.
    /// \throw  isx::ExceptionFileIO    If reading the cell set file fails.
    /// \throw  isx::ExceptionDataIO    If parsing the cell set file fails.
    CellSet(const std::string & inFileName);

    /// Write constructor.
    ///
    /// This opens a new file, writes header information.
    ///
    /// \param  inFileName      The name of the cell set file to write.
    /// \param  inTimingInfo    The timing information of the cell set.
    /// \param  inSpacingInfo   The spacing information of the cell set.
    /// \throw  isx::ExceptionFileIO    If writing the cell set file fails.
    /// \throw  isx::ExceptionDataIO    If formatting the cell set data fails.
    CellSet(const std::string & inFileName,
            const TimingInfo & inTimingInfo,
            const SpacingInfo & inSpacingInfo);

    /// \return True if the cell set file is valid, false otherwise.
    ///
    bool isValid() const;

    /// \return     The name of the file.
    ///
    std::string getFileName() const;

    /// \return     The number of cells contained in the file
    ///
    const isize_t getNumCells();

    /// \return     The timing information read from the cell set.
    ///
    isx::TimingInfo getTimingInfo() const;

    /// \return     The spacing information read from the cell set.
    ///
    isx::SpacingInfo getSpacingInfo() const;

    /// \return             A shared pointer to the trace data of the indexed cell.
    /// \param  inIndex     The index of the cell
    /// \throw  isx::ExceptionFileIO    If cell does not exist or reading fails.
    SpFTrace_t getTrace(isize_t inIndex);

    /// \return             A shared pointer to the image data of the indexed cell
    /// \param  inIndex     The index of the cell.
    /// \throw  isx::ExceptionFileIO    If cell does not exist or reading fails.
    SpFImage_t getImage(isize_t inIndex);

    /// Write image and trace cell data.
    ///
    /// If the cell already exists, it will overwrite its data.
    /// Otherwise, it will be appended.
    ///
    /// \param  inIndex     The index of the cell.
    /// \param  inImage     The cell image data to write.
    /// \param  inTrace     The cell trace data to write.
    /// \throw  isx::ExceptionFileIO    If trying to access unexistent cell or writing fails.
    void writeCellData(
            isize_t inIndex,
            Image<float> & inImage,
            Trace<float> & inTrace);

    /// \return             True if the cell is valid
    /// \param  inIndex     The index of the cell.
    /// \throw  isx::ExceptionFileIO    If trying to access unexistent cell or reading fails.
    bool isCellValid(isize_t inIndex);

    /// Set a cell in the set to be valid/invalid.
    ///
    /// This is used for rejecting or accepting a cell.
    ///
    /// \param inIndex the cell of interest
    /// \param inIsValid whether to reject or accept a cell in the set
    /// \throw  isx::ExceptionFileIO    If trying to access unexistent cell or reading fails.
    void setCellValid(isize_t inIndex, bool inIsValid);

private:

    /// True if the cell set is valid, false otherwise.
    bool m_valid = false;

    /// The cell set file to read/write from/to.
    std::unique_ptr<CellSetFile> m_file;

};

}
#endif // ISX_CELL_SET_H
