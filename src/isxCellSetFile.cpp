#include "isxCellSetFile.h"
#include "isxImage.h"
#include "isxException.h"
#include "isxAssert.h"
#include "isxPathUtils.h"

#include <cmath>

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
                const SpacingInfo & inSpacingInfo,
                const bool inIsRoiSet)
                : m_fileName(inFileName)
                , m_timingInfo(inTimingInfo)
                , m_spacingInfo(inSpacingInfo)
                , m_isRoiSet(inIsRoiSet)
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

            isx::closeFileStreamWithChecks(m_file, m_fileName);
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
            ISX_LOG_ERROR("Unknown exception closing file ", m_fileName);
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
    CellSetFile::writeCellData(isize_t inCellId, const Image & inSegmentationImage, Trace<float> & inData, const std::string & inName)
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

        if (inCellId == m_numCells)
        {
            m_cellNames.push_back(inName);
            m_cellStatuses.push_back(CellSet::CellStatus::UNDECIDED);
            m_cellColors.push_back(Color());
            m_cellActivity.push_back(true);
            if (hasMetrics())
            {
                m_cellImageMetrics.push_back(SpImageMetrics_t());
            }

            ++m_numCells;
        }
        else if (inCellId < m_numCells)
        {
            // Overwrite existing cell
            seekToCell(inCellId);

            m_cellNames.at(inCellId) = inName;
            m_cellStatuses.at(inCellId) = CellSet::CellStatus::UNDECIDED;
            m_cellColors.at(inCellId) = Color();
            m_cellActivity.at(inCellId) = true;
            if (hasMetrics())
            {
                m_cellImageMetrics.at(inCellId) = SpImageMetrics_t();
            }
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

    Color
    CellSetFile::getCellColor(isize_t inCellId)
    {
        return m_cellColors.at(inCellId);
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

    void
    CellSetFile::setCellColor(isize_t inCellId, const Color& inColor)
    {
        m_cellColors.at(inCellId) = inColor;
        if (m_openmode & std::ios_base::out)
        {
            if (m_fileClosedForWriting)
            {
                ISX_THROW(ExceptionFileIO, "Writing data after file was closed for writing.", m_fileName);
            }
            else
            {
                writeHeader();
            }
        }
    }

    void
    CellSetFile::setCellColors(const IdColorPairs& inColor)
    {
        for (auto & c : inColor)
        {
            m_cellColors.at(c.first) = c.second;
        }

        if (m_openmode & std::ios_base::out)
        {
            if (m_fileClosedForWriting)
            {
                ISX_THROW(ExceptionFileIO, "Writing data after file was closed for writing.", m_fileName);
            }
            else
            {
                writeHeader();
            }
        }
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

    bool
    CellSetFile::isCellActive(isize_t inCellId) const
    {
        return m_cellActivity.at(inCellId);
    }

    void
    CellSetFile::setCellActive(isize_t inCellId, bool inActive)
    {
        if (m_fileClosedForWriting)
        {
            ISX_THROW(isx::ExceptionFileIO,
                      "Writing data after file was closed for writing.", m_fileName);
        }
        m_cellActivity.at(inCellId) = inActive;
    }


    bool
    CellSetFile::isRoiSet() const
    {
        return m_isRoiSet;
    }

    isize_t
    CellSetFile::getSizeGlobalCS()
    {
        return m_sizeGlobalCS;
    }

    void
    CellSetFile::setSizeGlobalCS(const isize_t inSizeGlobalCS)
    {
        m_sizeGlobalCS = inSizeGlobalCS;
    }

    std::vector<int16_t>
    CellSetFile::getMatches()
    {
        return m_matches;
    }

    void
    CellSetFile::setMatches(const std::vector<int16_t> & inMatches)
    {
        m_matches = inMatches;
    }

    std::vector<double>
    CellSetFile::getPairScores()
    {
        return m_pairScores;
    }

    void
    CellSetFile::setPairScores(const std::vector<double> & inPairScores)
    {
        m_pairScores = inPairScores;
    }

    std::vector<double>
    CellSetFile::getCentroidDistances()
    {
        return m_centroidDistances;
    }

    void
    CellSetFile::setCentroidDistances(const std::vector<double> & inCentroidDistances)
    {
        m_centroidDistances = inCentroidDistances;
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

            auto version = j["fileVersion"].get<size_t>();
            switch (version)
            {
            case 4: 
                m_cellImageMetrics = convertJsonToCellMetrics(j["cellMetrics"]);
            case 3:
                m_sizeGlobalCS = j["SizeGlobalCS"];
                m_matches = j["Matches"].get<std::vector<int16_t>>();
                m_pairScores = j["PairScores"].get<std::vector<double>>();
                m_centroidDistances = j["CentroidDistances"].get<std::vector<double>>();
            case 2:
                m_cellColors = convertJsonToCellColors(j["CellColors"]);
            case 1:
                m_cellActivity = convertJsonToCellActivities(j["CellActivity"]);
            case 0:
            default:
                m_timingInfo = convertJsonToTimingInfo(j["timingInfo"]);
                m_spacingInfo = convertJsonToSpacingInfo(j["spacingInfo"]);
                m_cellNames = convertJsonToCellNames(j["CellNames"]);
                m_cellStatuses = convertJsonToCellStatuses(j["CellStatuses"]);
                m_isRoiSet = j["isRoiSet"];
                break;
            }

            if (m_cellActivity.empty())
            {
                m_cellActivity = std::vector<bool>(m_cellNames.size(), true);
            }
            if (m_cellColors.empty())
            {
                m_cellColors = CellColors_t(m_cellNames.size(), Color());
            }

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
        if (m_numCells != m_cellNames.size() || m_numCells != m_cellStatuses.size()  || m_numCells != m_cellColors.size() )
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
            replaceEmptyNames();
            j["CellNames"] = convertCellNamesToJson(m_cellNames);
            j["CellStatuses"] = convertCellStatusesToJson(m_cellStatuses);
            j["CellColors"] = convertCellColorsToJson(m_cellColors);
            j["producer"] = getProducerAsJson();
            j["fileVersion"] = s_version;
            j["isRoiSet"] = m_isRoiSet;
            j["CellActivity"] = convertCellActivitiesToJson(m_cellActivity);
            j["SizeGlobalCS"] = m_sizeGlobalCS;
            j["Matches"] = m_matches;
            j["PairScores"] = m_pairScores;
            j["CentroidDistances"] = m_centroidDistances;
            j["cellMetrics"] = convertCellMetricsToJson(m_cellImageMetrics);
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

    void
    CellSetFile::replaceEmptyNames()
    {
        size_t width = 1;
        if (m_numCells > 10)
        {
            width = size_t(std::floor(std::log10(m_numCells - 1)) + 1);
        }
        for (isize_t i = 0; i < m_numCells; ++i)
        {
            if (m_cellNames[i].empty())
            {
                m_cellNames[i] = "C" + convertNumberToPaddedString(i, width);
            }
        }
    }

    bool 
    CellSetFile::hasMetrics() const
    {
        return !m_cellImageMetrics.empty();
    }

    SpImageMetrics_t 
    CellSetFile::getImageMetrics(isize_t inIndex) const
    {
        if (m_cellImageMetrics.size() > inIndex)
        {
            return m_cellImageMetrics.at(inIndex);
        }
        return SpImageMetrics_t();
    }

    void
    CellSetFile::setImageMetrics(isize_t inIndex, const SpImageMetrics_t & inMetrics)
    {
        if (m_fileClosedForWriting)
        {
            ISX_THROW(isx::ExceptionFileIO,
                      "Writing data after file was closed for writing.", m_fileName);
        }

        if (!hasMetrics())
        {
            m_cellImageMetrics = CellMetrics_t(m_cellNames.size(), SpImageMetrics_t());
        }
        m_cellImageMetrics.at(inIndex) = inMetrics;
    }
}
