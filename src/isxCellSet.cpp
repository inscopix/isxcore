#include "isxCellSet.h"
#include "isxCellSetFile.h"

namespace isx
{

CellSet::CellSet(const std::string & inFileName)
{
    m_file = std::unique_ptr<CellSetFile>(new CellSetFile(inFileName));
    m_valid = true;
}

CellSet::CellSet(
        const std::string & inFileName,
        const TimingInfo & inTimingInfo,
        const SpacingInfo & inSpacingInfo)
{
    m_file = std::unique_ptr<CellSetFile>(new CellSetFile(
                inFileName, inTimingInfo, inSpacingInfo));
    m_valid = true;
}

bool
CellSet::isValid() const
{
    return m_valid;
}

std::string
CellSet::getFileName() const
{
    return m_file->getFileName();
}

const isize_t
CellSet::getNumCells()
{
    return m_file->numberOfCells();
}

isx::TimingInfo
CellSet::getTimingInfo() const
{
    return m_file->getTimingInfo();
}

isx::SpacingInfo
CellSet::getSpacingInfo() const
{
    return m_file->getSpacingInfo();
}

SpFTrace_t
CellSet::getTrace(isize_t inIndex)
{
    // NOTE sweet : this is currently performed on the calling thread.
    // We may want to change this to be on the IO thread.
    return m_file->readTrace(inIndex);
}

SpFImage_t
CellSet::getImage(isize_t inIndex)
{
    // NOTE sweet : this is currently performed on the calling thread.
    // We may want to change this to be on the IO thread.
    return m_file->readSegmentationImage(inIndex);
}

void
CellSet::writeCellData(
        isize_t inIndex,
        Image<float> & inImage,
        Trace<float> & inTrace)
{
    m_file->writeCellData(inIndex, inImage, inTrace);
}

bool
CellSet::isCellValid(isize_t inIndex)
{
    return m_file->isCellValid(inIndex);
}

void
CellSet::setCellValid(isize_t inIndex, bool inIsValid)
{
    return m_file->setCellValid(inIsValid, inIsValid);
}

}
