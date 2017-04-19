#include "isxCellSetFile.h"
#include "isxImage.h"
#include "isxException.h"
#include "isxAssert.h"

namespace isx
{
    CellSetFile::CellSetFile()
    {
        
    }

    CellSetFile::CellSetFile(const std::string & inFileName, bool enableWrite) : 
        m_fileName(inFileName)
    {
        m_openmode = std::ios::binary | std::ios_base::in;

        if (enableWrite)
        {
            m_openmode |= std::ios_base::out;
        }

        m_file.open(m_fileName, m_openmode);
        if (!m_file.good() || !m_file.is_open())
        {
            ISX_THROW(isx::ExceptionFileIO,
                "Failed to open cell set file for reading: ", m_fileName);
        }
        readHeader();
        m_fileClosedForWriting = !enableWrite;
        m_valid = true;
    }

    CellSetFile::CellSetFile(const std::string & inFileName, 
                const TimingInfo & inTimingInfo,
                const SpacingInfo & inSpacingInfo) 
                : m_fileName(inFileName)
                , m_timingInfo(inTimingInfo)
                , m_spacingInfo(inSpacingInfo)
    {
        m_openmode = std::ios::binary | std::ios_base::in | std::ios_base::out | std::ios::trunc;
        m_file.open(m_fileName, m_openmode);
        if (!m_file.good() || !m_file.is_open())
        {
            ISX_THROW(isx::ExceptionFileIO,
                "Failed to open cell set file for read/write: ", m_fileName);
        }
        m_valid = true;
    }

    CellSetFile::~CellSetFile()
    {
        if (isValid())
        {
            ISX_ASSERT(m_fileClosedForWriting);
            if (!m_fileClosedForWriting)
            {
                closeForWriting();
            }
            
            if(m_file.is_open() && m_file.good())
            {
                m_file.close();
                if (!m_file.good())
                {
                    ISX_LOG_ERROR("Error closing the stream for file", m_fileName,
                    " eof: ", m_file.eof(), 
                    " bad: ", m_file.bad(), 
                    " fail: ", m_file.fail());
                }
            }
        }
    }
    
    void
    CellSetFile::closeForWriting()
    {
        try
        {
            if (!m_fileClosedForWriting)
            {
                writeHeader();
                m_fileClosedForWriting = true;
            }
        }
        catch(isx::Exception &)
        {
        }
        catch(std::exception & e)
        {
            ISX_LOG_ERROR("Exception closing file ", m_fileName, ": ", e.what());
        }
        catch(...)
        {
            ISX_LOG_ERROR("Unkown exception closing file ", m_fileName);
        }
    }

    bool 
    CellSetFile::isValid() const 
    {
        return m_valid;
    }

    std::string 
    CellSetFile::getFileName() const 
    {
        return m_fileName;
    }

    const isize_t 
    CellSetFile::numberOfCells() 
    {
        return m_numCells;
    }

    const isx::TimingInfo & 
    CellSetFile::getTimingInfo() const 
    {
        return m_timingInfo;
    }

    const isx::SpacingInfo & 
    CellSetFile::getSpacingInfo() const
    {
        return m_spacingInfo;
    }

    SpFTrace_t 
    CellSetFile::readTrace(isize_t inCellId)      
    {
        seekToCell(inCellId);      
        
        // Calculate bytes till beginning of cell data
        isize_t offsetInBytes = segmentationImageSizeInBytes();
        
        m_file.seekg(offsetInBytes, std::ios_base::cur);
        if (!m_file.good())
        {
            ISX_THROW(isx::ExceptionFileIO, "Error seeking to cell trace for read.");
        }
        
        // Prepare data vector
        SpFTrace_t trace = std::make_shared<FTrace_t>(m_timingInfo);        
        m_file.read(reinterpret_cast<char*>(trace->getValues()), traceSizeInBytes());        
        if (!m_file.good())
        {
            ISX_THROW(isx::ExceptionFileIO, "Error reading cell trace.");
        }
        return trace;
        
    }
    
    SpImage_t 
    CellSetFile::readSegmentationImage(isize_t inCellId) 
    {
        
        seekToCell(inCellId);    
        
        // "Calculate" bytes till beginning of the segmentation image
        isize_t offsetInBytes = 0;
        
        m_file.seekg(offsetInBytes, std::ios_base::cur);
        if (!m_file.good())
        {
            ISX_THROW(isx::ExceptionFileIO, "Error seeking to cell segmentation image.");
        }        

        SpImage_t image = std::make_shared<Image>(
                m_spacingInfo,
                sizeof(float) * m_spacingInfo.getNumColumns(),
                1,
                DataType::F32);
        m_file.read(image->getPixels(), image->getImageSizeInBytes());
        
        if (!m_file.good())
        {
            ISX_THROW(isx::ExceptionFileIO, "Error reading cell segmentation image.");
        }

        return image;
    }
    
    void 
    CellSetFile::writeCellData(isize_t inCellId, Image & inSegmentationImage, Trace<float> & inData, const std::string & inName)
    {
        if (m_fileClosedForWriting)
        {
            ISX_THROW(isx::ExceptionFileIO,
                      "Writing data after file was closed for writing.", m_fileName);
        }

        // Check that image is F32
        const DataType dataType = inSegmentationImage.getDataType();
        if (dataType != DataType::F32)
        {
            ISX_THROW(isx::ExceptionDataIO,
                    "Expected F32 data type, instead got: ", dataType);
        }

        // Input validity
        isize_t inImageSizeInBytes = inSegmentationImage.getImageSizeInBytes();
        isize_t fImageSizeInBytes = segmentationImageSizeInBytes();
        ISX_ASSERT(inImageSizeInBytes == fImageSizeInBytes);
        
        isize_t inSamples = inData.getTimingInfo().getNumTimes();
        isize_t fSamples = m_timingInfo.getNumTimes();
        ISX_ASSERT(inSamples == fSamples);
        
        auto name = inName != "" ? inName : "C" + std::to_string(inCellId);
        
        if (inCellId == m_numCells)
        {
            m_cellNames.push_back(name);
            m_cellStatuses.push_back(CellSet::CellStatus::UNDECIDED);

            ++m_numCells;
        }
        else if (inCellId < m_numCells)
        {
            // Overwrite existing cell
            seekToCell(inCellId);
 
            m_cellNames.at(inCellId) = name;
            m_cellStatuses.at(inCellId) = CellSet::CellStatus::UNDECIDED;
        }
        else
        {
            ISX_THROW(isx::ExceptionDataIO,
                      "Writing cell indexes out of order is unsupported.");
        }

        m_file.write(inSegmentationImage.getPixels(), inImageSizeInBytes);
        m_file.write(reinterpret_cast<char*>(inData.getValues()), traceSizeInBytes());
        if (!m_file.good())
        {
            ISX_THROW(isx::ExceptionFileIO,
                "Failed to write cell data to file: ", m_fileName);
        }
        m_headerOffset = m_file.tellp();
        flush();
    }

    CellSet::CellStatus 
    CellSetFile::getCellStatus(isize_t inCellId) 
    {
        return m_cellStatuses.at(inCellId);
    }

    std::string
    CellSetFile::getCellStatusString(isize_t inCellId)
    {
        switch (m_cellStatuses.at(inCellId))
        {
            case CellSet::CellStatus::ACCEPTED:
                return "accepted";
            case CellSet::CellStatus::REJECTED:
                return "rejected";
            case CellSet::CellStatus::UNDECIDED:
                return "undecided";
        }

        return "";
    }

    void 
    CellSetFile::setCellStatus(isize_t inCellId, CellSet::CellStatus inStatus)
    {
        if (m_fileClosedForWriting)
        {
            ISX_THROW(isx::ExceptionFileIO,
                      "Writing data after file was closed for writing.", m_fileName);
        }
        m_cellStatuses.at(inCellId) = inStatus;
        writeHeader();
    }

    std::string 
    CellSetFile::getCellName(isize_t inCellId) 
    {
        return m_cellNames.at(inCellId);
    }

    void 
    CellSetFile::setCellName(isize_t inCellId, const std::string & inName)
    {
        if (m_fileClosedForWriting)
        {
            ISX_THROW(isx::ExceptionFileIO,
                      "Writing data after file was closed for writing.", m_fileName);
        }
        m_cellNames.at(inCellId) = inName;
    }
    
    void 
    CellSetFile::readHeader()
    {
        json j = readJsonHeaderAtEnd(m_file, m_headerOffset);

        try
        {
            std::string dataType = j["dataType"];
            DataSet::Type type = DataSet::Type(size_t(j["type"]));
            if (type != DataSet::Type::CELLSET)
            {
                ISX_THROW(isx::ExceptionDataIO,
                        "Expected type to be CellSet. Instead got ", size_t(type), ".");
            }
            m_timingInfo = convertJsonToTimingInfo(j["timingInfo"]);
            m_spacingInfo = convertJsonToSpacingInfo(j["spacingInfo"]);
            m_cellNames = convertJsonToCellNames(j["CellNames"]);
            m_cellStatuses = convertJsonToCellStatuses(j["CellStatuses"]);
        }
        catch (const std::exception & error)
        {
            ISX_THROW(isx::ExceptionDataIO, "Error parsing cell set header: ", error.what());
        }
        catch (...)
        {
            ISX_THROW(isx::ExceptionDataIO, "Unknown error while parsing cell set header.");
        }

        const isize_t bytesPerCell = segmentationImageSizeInBytes() + traceSizeInBytes();
        m_numCells = isize_t(m_headerOffset) / bytesPerCell;
        if (m_numCells != m_cellNames.size() || m_numCells != m_cellStatuses.size())
        {
            ISX_THROW(isx::ExceptionDataIO, "Number of cells in header does not match number of cells in file.");
        }
    }

    void 
    CellSetFile::writeHeader()
    {
        json j;
        try
        {
            j["type"] = size_t(DataSet::Type::CELLSET);
            j["dataType"] = "float";
            j["timingInfo"] = convertTimingInfoToJson(m_timingInfo);
            j["spacingInfo"] = convertSpacingInfoToJson(m_spacingInfo);
            j["mosaicVersion"] = CoreVersionVector();
            j["CellNames"] = convertCellNamesToJson(m_cellNames);
            j["CellStatuses"] = convertCellStatusesToJson(m_cellStatuses);
        }
        catch (const std::exception & error)
        {
            ISX_THROW(isx::ExceptionDataIO,
                "Error generating cell set header: ", error.what());
        }
        catch (...)
        {
            ISX_THROW(isx::ExceptionDataIO,
                "Unknown error while generating cell set header.");
        }

        if (m_numCells > 0)
        {
            seekToCell(m_numCells - 1);
            isize_t cellSize = segmentationImageSizeInBytes() + traceSizeInBytes();
            m_file.seekp(cellSize, std::ios_base::cur);
        }
        
        m_headerOffset = m_file.tellp();
        writeJsonHeaderAtEnd(j, m_file);

        flush();
    }
    
    void 
    CellSetFile::seekToCell(isize_t inCellId)
    {
        isize_t pos = 0;
        if (inCellId >= m_numCells)
        {
            ISX_THROW(isx::ExceptionFileIO,
                "Unable to seek to cell ID ", inCellId, " in file: ", m_fileName);
        }        
        
        isize_t cellSize = segmentationImageSizeInBytes() + traceSizeInBytes();
        
        pos += cellSize * inCellId;

        m_file.seekg(pos, std::ios_base::beg);
        
        if (!m_file.good() || pos >= isize_t(m_headerOffset))
        {
            ISX_THROW(isx::ExceptionFileIO, "Error reading cell id.");
        }

        if (m_openmode & std::ios_base::out)
        {
            m_file.seekp(m_file.tellg(), std::ios_base::beg);   // Make sure write and read pointers are tied together
        }
    }

    isize_t
    CellSetFile::segmentationImageSizeInBytes()
    {
        return m_spacingInfo.getTotalNumPixels() * sizeof(float);
    }
    
    isize_t 
    CellSetFile::traceSizeInBytes()
    {
        return m_timingInfo.getNumTimes() * sizeof(float);
    }

    void CellSetFile::flush()
    {
        m_file.flush();

        if (!m_file.good())
        {
            ISX_THROW(isx::ExceptionFileIO, "Error flushing the file stream.");
        }
    }
    
    
}
