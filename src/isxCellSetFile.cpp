#include "isxCellSetFile.h"
#include "isxImage.h"
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
        m_openmode = std::ios::binary | std::ios_base::in;
        m_file.open(m_fileName, m_openmode);
        if (!m_file.good() || !m_file.is_open())
        {
            ISX_THROW(isx::ExceptionFileIO,
                "Failed to open cell set file for reading: ", m_fileName);
        }
        readHeader();
        readCellNames();
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
        writeHeader();
        m_valid = true;            
    }

    CellSetFile::~CellSetFile()
    {
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
        isize_t offsetInBytes = cellValiditySizeInBytes() + cellNameSizeInBytes() + reservedSizeInBytes() + segmentationImageSizeInBytes();
        
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
        
        // Calculate bytes till beginning of the segmentation image
        isize_t offsetInBytes = cellValiditySizeInBytes() + cellNameSizeInBytes() + reservedSizeInBytes();
        
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
        
           
        if (inCellId >= m_numCells)
        {
            // Append cell data            
            uint32_t nextCellId = (uint32_t)m_numCells;
            m_file.seekp(0, std::ios_base::end);
            m_file.write((char *) &nextCellId, sizeof(uint32_t));
            if (!m_file.good())
            {
                ISX_THROW(isx::ExceptionFileIO,
                    "Failed to write cell ID: ", m_fileName);
            }

            m_cellNames.push_back(std::string());
            
            ++m_numCells;
        }
        else
        {
            // Overwrite existing cell
            seekToCell(inCellId);            
        }
        
        char valid = 1;
        m_file.write(&valid, sizeof(char));

        // Write cell name
        std::string name = inName;
        if(inName.empty())
        {
            name = "C" + std::to_string(inCellId);
        }
        else if(inName.size() > 15)
        {
            name = inName.substr(0, 15);
        }

        m_cellNames[inCellId] = name;
        
        while(name.size() < 16)
        {
            name += '\0';
        }

        m_file.write(name.data(), name.length());
        char * zeroBuf = new char[reservedSizeInBytes()];
        m_file.write(zeroBuf, reservedSizeInBytes());
        delete [] zeroBuf;

        m_file.write(inSegmentationImage.getPixels(), inImageSizeInBytes);
        m_file.write(reinterpret_cast<char*>(inData.getValues()), traceSizeInBytes());
        if (!m_file.good())
        {
            ISX_THROW(isx::ExceptionFileIO,
                "Failed to write cell data to file: ", m_fileName);
        }
        flush();
    }

    bool 
    CellSetFile::isCellValid(isize_t inCellId) 
    {
        
        seekToCell(inCellId); 
        
        char isValid = 0;
        m_file.read(&isValid, sizeof(char));
        if (!m_file.good())
        {
            ISX_THROW(isx::ExceptionFileIO, "Error reading cell information.");
        }
        return isValid == 1 ? true : false;
    }
    
    void 
    CellSetFile::setCellValid(isize_t inCellId, bool inIsValid)
    {
        seekToCell(inCellId); 
                
        char isValid = inIsValid ? 1 : 0;
        m_file.write((char *)&isValid, sizeof(char));
        if (!m_file.good())
        {
            ISX_THROW(isx::ExceptionFileIO, "Error writing cell information.");
        } 
        flush();
    }

    std::string 
    CellSetFile::getCellName(isize_t inCellId) 
    {
        return m_cellNames[inCellId];
    }

    void 
    CellSetFile::setCellName(isize_t inCellId, const std::string & inName)
    {
       
        // Overwrite existing cell
        seekToCell(inCellId); 
                
        // Calculate bytes till beginning of the segmentation image
        isize_t offsetInBytes = cellValiditySizeInBytes();
        
        m_file.seekp(offsetInBytes, std::ios_base::cur);
        if (!m_file.good())
        {
            ISX_THROW(isx::ExceptionFileIO, "Error seeking to cell name.");
        }

        // Write cell name
        std::string name = inName;
        if(inName.empty())
        {
            name = "C" + std::to_string(inCellId);
        }
        else if(inName.size() > 15)
        {
            name = inName.substr(0, 15);
        }

        m_cellNames[inCellId] = name;

        name += '\0';

        m_file.write(name.data(), name.length());
        if (!m_file.good())
        {
            ISX_THROW(isx::ExceptionFileIO, "Error writing cell name.");
        } 
        flush();
        
    }
    
    void 
    CellSetFile::readHeader()
    {
        json j = readJsonHeader(m_file);
        m_headerOffset = m_file.tellg();

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

        m_file.seekg( 0, std::ios::end);
        std::streamoff offsetInCells = m_file.tellg() - m_headerOffset;
        isize_t bytesInCells = 0;

        if(offsetInCells > 0)
        {
            bytesInCells = (isize_t) offsetInCells;
        }

        isize_t bytesPerCell = cellHeaderSizeInBytes() + traceSizeInBytes();
        m_numCells = bytesInCells / bytesPerCell;
    }

    void 
    CellSetFile::readCellNames() 
    {
        if (m_numCells != 0)
        {
            m_cellNames.resize(m_numCells);

            for (isize_t id(0); id < m_numCells; ++id)
            {
                m_cellNames[id] = readCellName(id);
            }
        }
    }

    std::string 
    CellSetFile::readCellName(isize_t inCellId)
    {
        seekToCell(inCellId);

        // Calculate bytes till beginning of the segmentation image
        isize_t offsetInBytes = cellValiditySizeInBytes();

        m_file.seekg(offsetInBytes, std::ios_base::cur);
        if (!m_file.good())
        {
            ISX_THROW(isx::ExceptionFileIO, "Error seeking to cell name.");
        }

        char name[16];
        m_file.read(name, 16);

        if (!m_file.good())
        {
            ISX_THROW(isx::ExceptionFileIO, "Error reading cell name.");
        }

        std::string strName(name);

        return strName;
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

        writeJsonHeader(j, m_file);
        m_headerOffset = m_file.tellp();
        flush();
    }
    
    void 
    CellSetFile::seekToCell(isize_t inCellId)
    {
        isize_t pos = m_headerOffset;
        
        if(inCellId >= m_numCells)
        {
            ISX_THROW(isx::ExceptionFileIO,
                "Unable to seek to cell ID ", inCellId, " in file: ", m_fileName);
        }        
        
        isize_t cellSize = cellHeaderSizeInBytes() + traceSizeInBytes();
        
        pos += cellSize * inCellId;

        m_file.seekg(pos, std::ios_base::beg);
        
        uint32_t currentId;
        m_file.read((char *) &currentId, sizeof(uint32_t));
        if (!m_file.good())
        {
            ISX_THROW(isx::ExceptionFileIO, "Error reading cell id.");
        }
        ISX_ASSERT(currentId == (uint32_t)inCellId);   

        if(m_openmode & std::ios_base::out)
        {
            m_file.seekp(m_file.tellg(), std::ios_base::beg);   // Make sure write and read pointers are tied together
        }

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
    CellSetFile::cellNameSizeInBytes()
    {
        return (sizeof(char)*16);
    }
    
    isize_t 
    CellSetFile::reservedSizeInBytes()
    {
        return (128 - (cellIdSizeInBytes() + cellValiditySizeInBytes() + cellNameSizeInBytes()));
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
        isize_t bytes = 128 + segmentationImageSizeInBytes();
        return bytes;
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
