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
        if (m_valid)
        {
            // Overwrite the header to update the total number of cells
            writeHeader(false);
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
        std::fstream file(m_fileName, std::ios::binary | std::ios_base::in);
        if (!file.good())
        {
            ISX_THROW(isx::ExceptionFileIO,
                "Failed to open cell set file for reading: ", m_fileName);
        }
        seekToCell(inCellId, file);      
        
        // Calculate bytes till beginning of cell data
        SizeInPixels_t nPixels = m_spacingInfo.getTotalNumPixels();
        isize_t offsetInBytes = sizeof(char) + sizeof(float) * nPixels;
        
        file.seekg(offsetInBytes, std::ios_base::cur);
        if (!file.good())
        {
            ISX_THROW(isx::ExceptionFileIO, "Error seeking to cell trace for read.");
        }
        
        // Prepare data vector
        SpFTrace_t trace = std::make_shared<FTrace_t>(m_timingInfo);        
        file.read(reinterpret_cast<char*>(trace->getValues()), m_timingInfo.getNumTimes() * sizeof(float));        
        if (!file.good())
        {
            ISX_THROW(isx::ExceptionFileIO, "Error reading cell trace.");
        }
        return trace;
        
    }
    
    SpFImage_t 
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
        isize_t offsetInBytes = sizeof(char);
        
        file.seekg(offsetInBytes, std::ios_base::cur);
        if (!file.good())
        {
            ISX_THROW(isx::ExceptionFileIO, "Error seeking to cell segmentation image.");
        }
        

        isize_t cols = m_spacingInfo.getNumColumns();
        SpFImage_t image = std::make_shared<FImage_t>(m_spacingInfo, cols * sizeof(float), 1);
        file.read(reinterpret_cast<char*>(image->getPixels()), image->getImageSizeInBytes());
        
        if (!file.good())
        {
            ISX_THROW(isx::ExceptionFileIO, "Error reading cell segmentation image.");
        }

        return image;
    }
    
    void 
    CellSetFile::writeCellData(isize_t inCellId, Image<float> & inSegmentationImage, const Trace<float> & inData)
    {
        // Input validity
        isize_t inImageSizeInBytes = inSegmentationImage.getImageSizeInBytes();
        isize_t fImageSizeInBytes = m_spacingInfo.getTotalNumPixels() * sizeof(float);
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
        file.write(reinterpret_cast<char*>(inData.getValues()), inSamples * sizeof(float));
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
        if (!file.good())
        {
            ISX_THROW(isx::ExceptionFileIO,
                "Error while reading cell set header: ", m_fileName);
        }

        std::string jsonStr;
        file.seekg(std::ios_base::beg);
        std::getline(file, jsonStr, '\0');
        if (!file.good())
        {
            ISX_THROW(isx::ExceptionFileIO,
                "Error while reading cell set header: ", m_fileName);
        }

        json j;
        try
        {
            j = json::parse(jsonStr);
        }
        catch (const std::exception & error)
        {
            ISX_THROW(isx::ExceptionDataIO, "Error while parsing cell set header: ", error.what());
        }
        catch (...)
        {
            ISX_THROW(isx::ExceptionDataIO, "Unknown error while parsing cell set header.");
        }

        m_headerOffset = file.tellg();

        try
        {
            std::string dataType = j["dataType"];
            std::string type = j["type"];
            if (type.compare("CellSet") != 0)
            {
                ISX_THROW(isx::ExceptionDataIO, "Expected type to be CellSet. Instead got ", type, ".");
            }
            m_timingInfo = convertJsonToTimingInfo(j["timingInfo"]);
            m_spacingInfo = convertJsonToSpacingInfo(j["spacingInfo"]);
            m_numCells = j["numCells"].get<isize_t>();
        }
        catch (const std::exception & error)
        {
            ISX_THROW(isx::ExceptionDataIO, "Error parsing cell set header: ", error.what());
        }
        catch (...)
        {
            ISX_THROW(isx::ExceptionDataIO, "Unknown error while parsing cell set header.");
        }
    }
    
    void 
    CellSetFile::writeHeader(bool inTruncate)
    {
        std::ios_base::openmode mode = std::ios::binary | std::ios::in | std::ios::out;
        
        if(inTruncate)
        {
            mode |= std::ios::trunc;
        }
        
        std::fstream file(m_fileName, mode);
        if (!file.good())
        {
            ISX_THROW(isx::ExceptionFileIO,
                "Failed to open cell set when writing header: ", m_fileName);
        }

        json j;
        try
        {
            j["type"] = "CellSet";
            j["dataType"] = "float";
            j["timingInfo"] = convertTimingInfoToJson(m_timingInfo);
            j["spacingInfo"] = convertSpacingInfoToJson(m_spacingInfo);
            j["numCells"] = m_numCells;
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
        
        file.seekg(std::ios_base::beg);
        file << std::setw(4) << j;
        file << '\0';
        if (!file.good())
        {
            ISX_THROW(isx::ExceptionFileIO,
                "Failed to write header in cell set file: ", m_fileName);
        }

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
        
        
        SizeInPixels_t nPixels = m_spacingInfo.getNumPixels();
        isize_t imageSizeInBytes = nPixels.getWidth() * nPixels.getHeight() * sizeof(float);
        isize_t samplesInBytes = m_timingInfo.getNumTimes() * sizeof(float);
        
        isize_t headerSize = sizeof(uint32_t) + sizeof(char) + imageSizeInBytes;
        isize_t cellSize = headerSize + samplesInBytes;
        
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
    
    
}