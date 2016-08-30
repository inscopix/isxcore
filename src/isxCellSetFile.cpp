#include "isxCellSetFile.h"
#include "isxException.h"
#include "isxAssert.h"
#include "isxJsonUtils.h"

namespace isx
{
    CellSetFile::CellSetFile()
    {
        
    }

    CellSetFile::CellSetFile(const std::string & inFileName) : 
        m_fileName(inFileName)
    {
        readHeader();
        m_valid = true;
    }

    CellSetFile::CellSetFile(const std::string & inFileName, 
                const TimingInfo & inTimingInfo,
                const SpacingInfo & inSpacingInfo) 
                : m_fileName(inFileName)
                , m_timingInfo(inTimingInfo)
                , m_spacingInfo(inSpacingInfo)
    {
        writeHeader(true);
        m_valid = true;            
    }

    CellSetFile::~CellSetFile()
    {
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
        std::fstream file(m_fileName, std::ios::binary | std::ios_base::in);
        if (!file.good())
        {
            ISX_THROW(isx::ExceptionFileIO,
                "Failed to open cell set file for reading: ", m_fileName);
        }
        seekToCell(inCellId, file);      
        
        // Calculate bytes till beginning of cell data
        isize_t offsetInBytes = cellValiditySizeInBytes() + segmentationImageSizeInBytes();
        
        file.seekg(offsetInBytes, std::ios_base::cur);
        if (!file.good())
        {
            ISX_THROW(isx::ExceptionFileIO, "Error seeking to cell trace for read.");
        }
        
        // Prepare data vector
        SpFTrace_t trace = std::make_shared<FTrace_t>(m_timingInfo);        
        file.read(reinterpret_cast<char*>(trace->getValues()), traceSizeInBytes());        
        if (!file.good())
        {
            ISX_THROW(isx::ExceptionFileIO, "Error reading cell trace.");
        }
        return trace;
        
    }
    
    SpImage_t 
    CellSetFile::readSegmentationImage(isize_t inCellId) 
    {
        std::fstream file(m_fileName, std::ios::binary | std::ios_base::in);
        if (!file.good())
        {
            ISX_THROW(isx::ExceptionFileIO,
                "Failed to open cell set file for reading: ", m_fileName);
        }
        seekToCell(inCellId, file);    
        
        // Calculate bytes till beginning of the segmentation image
        isize_t offsetInBytes = cellValiditySizeInBytes();
        
        file.seekg(offsetInBytes, std::ios_base::cur);
        if (!file.good())
        {
            ISX_THROW(isx::ExceptionFileIO, "Error seeking to cell segmentation image.");
        }        

        SpImage_t image = std::make_shared<Image>(
                m_spacingInfo,
                sizeof(float) * m_spacingInfo.getNumColumns(),
                1,
                DataType::F32);
        file.read(reinterpret_cast<char*>(image->getPixels()), image->getImageSizeInBytes());
        
        if (!file.good())
        {
            ISX_THROW(isx::ExceptionFileIO, "Error reading cell segmentation image.");
        }

        return image;
    }
    
    void 
    CellSetFile::writeCellData(isize_t inCellId, Image & inSegmentationImage, Trace<float> & inData)
    {
        // Input validity
        isize_t inImageSizeInBytes = inSegmentationImage.getImageSizeInBytes();
        isize_t fImageSizeInBytes = segmentationImageSizeInBytes();
        ISX_ASSERT(inImageSizeInBytes == fImageSizeInBytes);
        
        isize_t inSamples = inData.getTimingInfo().getNumTimes();
        isize_t fSamples = m_timingInfo.getNumTimes();
        ISX_ASSERT(inSamples == fSamples);
        
        // Open stream and write  
        std::fstream file;      
        
        
        if (inCellId >= m_numCells)
        {
            // Append cell data
            file.open(m_fileName, std::ios::binary | std::ios::app | std::ios::out);    // "Append" does not imply "out" in linux so it has to be explicitly set
            if (!file.is_open() || !file.good())
            {
                ISX_THROW(isx::ExceptionFileIO,
                    "Failed to open cell set file for append: ", m_fileName);
            }
            uint32_t nextCellId = (uint32_t)m_numCells;
            file.write((char *) &nextCellId, sizeof(uint32_t));
            if (!file.good())
            {
                ISX_THROW(isx::ExceptionFileIO,
                    "Failed to write cell ID: ", m_fileName);
            }
            
            ++m_numCells;
        }
        else
        {
            // Overwrite existing cell
            file.open(m_fileName, std::ios::binary | std::ios::in | std::ios::out);
            if (!file.good())
            {
                ISX_THROW(isx::ExceptionFileIO,
                    "Failed to open cell set file for writing: ", m_fileName);
            }
            seekToCell(inCellId, file);
        }
        
        char valid = 1;
        file.write(&valid, sizeof(char));
        file.write(reinterpret_cast<char*>(inSegmentationImage.getPixels()), inImageSizeInBytes);
        file.write(reinterpret_cast<char*>(inData.getValues()), traceSizeInBytes());
        if (!file.good())
        {
            ISX_THROW(isx::ExceptionFileIO,
                "Failed to write cell data to file: ", m_fileName);
        }
    }

    bool 
    CellSetFile::isCellValid(isize_t inCellId) 
    {
        std::fstream file(m_fileName, std::ios::binary | std::ios::in);
        if (!file.good())
        {
            ISX_THROW(isx::ExceptionFileIO,
                "Failed to open cell set file for reading: ", m_fileName);
        }
        seekToCell(inCellId, file); 
        
        char isValid = 0;
        file.read(&isValid, sizeof(char));
        if (!file.good())
        {
            ISX_THROW(isx::ExceptionFileIO, "Error reading cell information.");
        }
        return isValid == 1 ? true : false;
    }
    
    void 
    CellSetFile::setCellValid(isize_t inCellId, bool inIsValid)
    {
        std::fstream file(m_fileName, std::ios::binary | std::ios::in | std::ios::out);
        if (!file.good())
        {
            ISX_THROW(isx::ExceptionFileIO,
                "Failed to open cell set file for writing: ", m_fileName);
        }
        seekToCell(inCellId, file); 
        
        char isValid = inIsValid ? 1 : 0;
        file.write((char *)&isValid, sizeof(char));
        if (!file.good())
        {
            ISX_THROW(isx::ExceptionFileIO, "Error writing cell information.");
        } 
    }
    
    void 
    CellSetFile::readHeader()
    {
        std::fstream file(m_fileName, std::ios::binary | std::ios::in);
        json j = readJsonHeader(file);
        m_headerOffset = file.tellg();

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
        }
        catch (const std::exception & error)
        {
            ISX_THROW(isx::ExceptionDataIO, "Error parsing cell set header: ", error.what());
        }
        catch (...)
        {
            ISX_THROW(isx::ExceptionDataIO, "Unknown error while parsing cell set header.");
        }

        file.seekg( 0, std::ios::end);
        std::streamoff offsetInCells = file.tellg() - m_headerOffset;
        isize_t bytesInCells = 0;

        if(offsetInCells > 0)
        {
            bytesInCells = (isize_t) offsetInCells;
        }

        isize_t bytesPerCell = cellHeaderSizeInBytes() + traceSizeInBytes();
        m_numCells = bytesInCells / bytesPerCell;
    }
    
    void 
    CellSetFile::writeHeader(bool inTruncate)
    {
        json j;
        try
        {
            j["type"] = size_t(DataSet::Type::CELLSET);
            j["dataType"] = "float";
            j["timingInfo"] = convertTimingInfoToJson(m_timingInfo);
            j["spacingInfo"] = convertSpacingInfoToJson(m_spacingInfo);
            j["mosaicVersion"] = CoreVersionVector();
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

        std::ios_base::openmode mode = std::ios::binary | std::ios::out;
        if(inTruncate)
        {
            mode |= std::ios::trunc;
        }
        std::fstream file(m_fileName, mode);
        writeJsonHeader(j, file);
        m_headerOffset = file.tellp();
    }
    
    void 
    CellSetFile::seekToCell(isize_t inCellId, std::fstream &file)
    {
        isize_t pos = m_headerOffset;
        
        if(inCellId >= m_numCells)
        {
            ISX_THROW(isx::ExceptionFileIO,
                "Unable to seek to cell ID ", inCellId, " in file: ", m_fileName);
        }        
        
        isize_t cellSize = cellHeaderSizeInBytes() + traceSizeInBytes();
        
        pos += cellSize * inCellId;

        file.seekg(pos, std::ios_base::beg);
        
        uint32_t currentId;
        file.read((char *) &currentId, sizeof(uint32_t));
        if (!file.good())
        {
            ISX_THROW(isx::ExceptionFileIO, "Error reading cell id.");
        }
        ISX_ASSERT(currentId == (uint32_t)inCellId);
        file.seekp(file.tellg(), std::ios_base::beg);   // Make sure write and read pointers are tied together

    }

    isize_t 
    CellSetFile::cellIdSizeInBytes()
    {
        return sizeof(uint32_t);
    }

    isize_t 
    CellSetFile::cellValiditySizeInBytes()
    {
        return sizeof(char);
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

    
    isize_t 
    CellSetFile::cellHeaderSizeInBytes()
    {
        isize_t bytes = cellIdSizeInBytes() +  cellValiditySizeInBytes() + segmentationImageSizeInBytes();
        return bytes;
    }
    
    
}
