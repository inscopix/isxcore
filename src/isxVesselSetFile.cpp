#include "isxVesselSetFile.h"
#include "isxImage.h"
#include "isxException.h"
#include "isxAssert.h"
#include "isxPathUtils.h"

#include <cmath>

namespace isx
{
    VesselSetFile::VesselSetFile()
    {

    }

    VesselSetFile::VesselSetFile(const std::string & inFileName, bool enableWrite) :
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
                "Failed to open vessel set file for reading: ", m_fileName);
        }
        readHeader();
        m_fileClosedForWriting = !enableWrite;
        m_valid = true;
    }

    VesselSetFile::VesselSetFile(const std::string & inFileName,
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
                "Failed to open vessel set file for read/write: ", m_fileName);
        }
        m_valid = true;
    }

    VesselSetFile::~VesselSetFile()
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
    VesselSetFile::closeForWriting()
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
    VesselSetFile::isValid() const
    {
        return m_valid;
    }

    std::string
    VesselSetFile::getFileName() const
    {
        return m_fileName;
    }

    const isize_t
    VesselSetFile::numberOfVessels()
    {
        return m_numVessels;
    }

    const isx::TimingInfo &
    VesselSetFile::getTimingInfo() const
    {
        return m_timingInfo;
    }

    const isx::SpacingInfo &
    VesselSetFile::getSpacingInfo() const
    {
        return m_spacingInfo;
    }

    SpFTrace_t
    VesselSetFile::readTrace(isize_t inVesselId)
    {
        seekToVessel(inVesselId);

        // Calculate bytes till beginning of vessel data
        isize_t offsetInBytes = segmentationImageSizeInBytes();

        m_file.seekg(offsetInBytes, std::ios_base::cur);
        if (!m_file.good())
        {
            ISX_THROW(isx::ExceptionFileIO, "Error seeking to vessel trace for read.");
        }

        // Prepare data vector
        SpFTrace_t trace = std::make_shared<FTrace_t>(m_timingInfo);
        m_file.read(reinterpret_cast<char*>(trace->getValues()), traceSizeInBytes());
        if (!m_file.good())
        {
            ISX_THROW(isx::ExceptionFileIO, "Error reading vessel trace.");
        }
        return trace;

    }

    SpImage_t
    VesselSetFile::readSegmentationImage(isize_t inVesselId)
    {

        seekToVessel(inVesselId);

        // "Calculate" bytes till beginning of the segmentation image
        isize_t offsetInBytes = 0;

        m_file.seekg(offsetInBytes, std::ios_base::cur);
        if (!m_file.good())
        {
            ISX_THROW(isx::ExceptionFileIO, "Error seeking to vessel segmentation image.");
        }

        SpImage_t image = std::make_shared<Image>(
                m_spacingInfo,
                sizeof(float) * m_spacingInfo.getNumColumns(),
                1,
                DataType::F32);
        m_file.read(image->getPixels(), image->getImageSizeInBytes());

        if (!m_file.good())
        {
            ISX_THROW(isx::ExceptionFileIO, "Error reading vessel segmentation image.");
        }

        return image;
    }

    void
    VesselSetFile::writeVesselData(isize_t inVesselId, const Image & inSegmentationImage, Trace<float> & inData, const std::string & inName)
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

        if (inVesselId == m_numVessels)
        {
            m_vesselNames.push_back(inName);
            m_vesselStatuses.push_back(VesselSet::VesselStatus::UNDECIDED);
            m_vesselColors.push_back(Color());
            m_vesselActivity.push_back(true);
            ++m_numVessels;
        }
        else if (inVesselId < m_numVessels)
        {
            // Overwrite existing vessel
            seekToVessel(inVesselId);

            m_vesselNames.at(inVesselId) = inName;
            m_vesselStatuses.at(inVesselId) = VesselSet::VesselStatus::UNDECIDED;
            m_vesselColors.at(inVesselId) = Color();
            m_vesselActivity.at(inVesselId) = true;
        }
        else
        {
            ISX_THROW(isx::ExceptionDataIO,
                      "Writing vessel indexes out of order is unsupported.");
        }

        m_file.write(inSegmentationImage.getPixels(), inImageSizeInBytes);
        m_file.write(reinterpret_cast<char*>(inData.getValues()), traceSizeInBytes());
        if (!m_file.good())
        {
            ISX_THROW(isx::ExceptionFileIO,
                "Failed to write vessel data to file: ", m_fileName);
        }
        m_headerOffset = m_file.tellp();
        flush();
    }

    VesselSet::VesselStatus
    VesselSetFile::getVesselStatus(isize_t inVesselId)
    {
        return m_vesselStatuses.at(inVesselId);
    }

    Color
    VesselSetFile::getVesselColor(isize_t inVesselId)
    {
        return m_vesselColors.at(inVesselId);
    }

    std::string
    VesselSetFile::getVesselStatusString(isize_t inVesselId)
    {
        switch (m_vesselStatuses.at(inVesselId))
        {
            case VesselSet::VesselStatus::ACCEPTED:
                return "accepted";
            case VesselSet::VesselStatus::REJECTED:
                return "rejected";
            case VesselSet::VesselStatus::UNDECIDED:
                return "undecided";
        }

        return "";
    }

    void
    VesselSetFile::setVesselStatus(isize_t inVesselId, VesselSet::VesselStatus inStatus)
    {
        if (m_fileClosedForWriting)
        {
            ISX_THROW(isx::ExceptionFileIO,
                      "Writing data after file was closed for writing.", m_fileName);
        }
        m_vesselStatuses.at(inVesselId) = inStatus;
        writeHeader();
    }

    void
    VesselSetFile::setVesselColor(isize_t inVesselId, const Color& inColor)
    {
        m_vesselColors.at(inVesselId) = inColor;
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
    VesselSetFile::setVesselColors(const IdColorPairs& inColor)
    {
        for (auto & c : inColor)
        {
            m_vesselColors.at(c.first) = c.second;
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
    VesselSetFile::getVesselName(isize_t inVesselId)
    {
        return m_vesselNames.at(inVesselId);
    }

    void
    VesselSetFile::setVesselName(isize_t inVesselId, const std::string & inName)
    {
        if (m_fileClosedForWriting)
        {
            ISX_THROW(isx::ExceptionFileIO,
                      "Writing data after file was closed for writing.", m_fileName);
        }
        m_vesselNames.at(inVesselId) = inName;
    }

    bool
    VesselSetFile::isVesselActive(isize_t inVesselId) const
    {
        return m_vesselActivity.at(inVesselId);
    }

    void
    VesselSetFile::setVesselActive(isize_t inVesselId, bool inActive)
    {
        if (m_fileClosedForWriting)
        {
            ISX_THROW(isx::ExceptionFileIO,
                      "Writing data after file was closed for writing.", m_fileName);
        }
        m_vesselActivity.at(inVesselId) = inActive;
    }

    std::vector<uint16_t>
    VesselSetFile::getEfocusValues ()
    {
        return m_efocusValues;
    }

    void
    VesselSetFile::setEfocusValues(const std::vector<uint16_t> & inEfocus)
    {
        m_efocusValues = inEfocus;
    }

    void
    VesselSetFile::readHeader()
    {
        json j = readJsonHeaderAtEnd(m_file, m_headerOffset);

        try
        {
            std::string dataType = j["dataType"];
            DataSet::Type type = DataSet::Type(size_t(j["type"]));
            if (type != DataSet::Type::VESSELSET)
            {
                ISX_THROW(isx::ExceptionDataIO,
                        "Expected type to be VesselSet. Instead got ", size_t(type), ".");
            }

            auto version = j["fileVersion"].get<size_t>();
            m_timingInfo = convertJsonToTimingInfo(j["timingInfo"]);
            m_spacingInfo = convertJsonToSpacingInfo(j["spacingInfo"]);
            m_vesselNames = convertJsonToVesselNames(j["VesselNames"]);
            m_vesselStatuses = convertJsonToVesselStatuses(j["VesselStatuses"]);

            if (version >= 1)
            {
                m_vesselActivity = convertJsonToVesselActivities(j["VesselActivity"]);
            }

            if (version >= 2)
            {
                m_vesselColors = convertJsonToVesselColors(j["VesselColors"]);
            }

            if ((version >= 5) && j.find("extraProperties") != j.end())
            {
                m_extraProperties = j["extraProperties"];
            }

            if (j.find("efocusValues") != j.end())
            {
                m_efocusValues = j["efocusValues"].get<std::vector<uint16_t>>();
            }
            else if (m_extraProperties.find("idps") != m_extraProperties.end())
            {
                json idps = m_extraProperties["idps"];
                ISX_LOG_DEBUG(idps.dump());
                if (idps.find("efocus") != idps.end())
                {
                    ISX_LOG_DEBUG(idps["efocus"].get<uint16_t>());
                    m_efocusValues = {idps["efocus"].get<uint16_t>()};
                }
            }

            if (m_vesselActivity.empty())
            {
                m_vesselActivity = std::vector<bool>(m_vesselNames.size(), true);
            }

            if (m_vesselColors.empty())
            {
                m_vesselColors = VesselColors_t(m_vesselNames.size(), Color());
            }
        }
        catch (const std::exception & error)
        {
            ISX_THROW(isx::ExceptionDataIO, "Error parsing vessel set header: ", error.what());
        }
        catch (...)
        {
            ISX_THROW(isx::ExceptionDataIO, "Unknown error while parsing vessel set header.");
        }

        const isize_t bytesPerVessel = segmentationImageSizeInBytes() + traceSizeInBytes();
        m_numVessels = isize_t(m_headerOffset) / bytesPerVessel;
        if (m_numVessels != m_vesselNames.size() || m_numVessels != m_vesselStatuses.size()  || m_numVessels != m_vesselColors.size() )
        {
            ISX_THROW(isx::ExceptionDataIO, "Number of vessels in header does not match number of vessels in file.");
        }
    }

    void
    VesselSetFile::writeHeader()
    {
        json j;
        try
        {
            j["type"] = size_t(DataSet::Type::VESSELSET);
            j["dataType"] = "float";
            j["timingInfo"] = convertTimingInfoToJson(m_timingInfo);
            j["spacingInfo"] = convertSpacingInfoToJson(m_spacingInfo);
            replaceEmptyNames();
            j["VesselNames"] = convertVesselNamesToJson(m_vesselNames);
            j["VesselStatuses"] = convertVesselStatusesToJson(m_vesselStatuses);
            j["VesselColors"] = convertVesselColorsToJson(m_vesselColors);
            j["producer"] = getProducerAsJson();
            j["fileVersion"] = s_version;
            j["VesselActivity"] = convertVesselActivitiesToJson(m_vesselActivity);
            j["SizeGlobalVS"] = m_sizeGlobalVS;
            j["extraProperties"] = m_extraProperties;
            j["efocusValues"] = m_efocusValues;
        }
        catch (const std::exception & error)
        {
            ISX_THROW(isx::ExceptionDataIO,
                "Error generating vessel set header: ", error.what());
        }
        catch (...)
        {
            ISX_THROW(isx::ExceptionDataIO,
                "Unknown error while generating vessel set header.");
        }

        if (m_numVessels > 0)
        {
            seekToVessel(m_numVessels - 1);
            isize_t vesselSize = segmentationImageSizeInBytes() + traceSizeInBytes();
            m_file.seekp(vesselSize, std::ios_base::cur);
        }

        m_headerOffset = m_file.tellp();
        writeJsonHeaderAtEnd(j, m_file);

        flush();
    }

    void
    VesselSetFile::seekToVessel(isize_t inVesselId)
    {
        isize_t pos = 0;
        if (inVesselId >= m_numVessels)
        {
            ISX_THROW(isx::ExceptionFileIO,
                "Unable to seek to vessel ID ", inVesselId, " in file: ", m_fileName);
        }

        isize_t vesselSize = segmentationImageSizeInBytes() + traceSizeInBytes();

        pos += vesselSize * inVesselId;

        m_file.seekg(pos, std::ios_base::beg);

        if (!m_file.good() || pos >= isize_t(m_headerOffset))
        {
            ISX_THROW(isx::ExceptionFileIO, "Error reading vessel id.");
        }

        if (m_openmode & std::ios_base::out)
        {
            m_file.seekp(m_file.tellg(), std::ios_base::beg);   // Make sure write and read pointers are tied together
        }
    }

    isize_t
    VesselSetFile::segmentationImageSizeInBytes()
    {
        return m_spacingInfo.getTotalNumPixels() * sizeof(float);
    }

    isize_t
    VesselSetFile::traceSizeInBytes()
    {
        return m_timingInfo.getNumTimes() * sizeof(float);
    }

    void VesselSetFile::flush()
    {
        m_file.flush();

        if (!m_file.good())
        {
            ISX_THROW(isx::ExceptionFileIO, "Error flushing the file stream.");
        }
    }

    void
    VesselSetFile::replaceEmptyNames()
    {
        size_t width = 1;
        if (m_numVessels > 10)
        {
            width = size_t(std::floor(std::log10(m_numVessels - 1)) + 1);
        }
        for (isize_t i = 0; i < m_numVessels; ++i)
        {
            if (m_vesselNames[i].empty())
            {
                m_vesselNames[i] = "V" + convertNumberToPaddedString(i, width);
            }
        }
    }

    std::string
    VesselSetFile::getExtraProperties() const
    {
        return m_extraProperties.dump();
    }

    void
    VesselSetFile::setExtraProperties(const std::string & inProperties)
    {
        try
        {
            m_extraProperties = json::parse(inProperties);
        }
        catch (const std::exception & error)
        {
            ISX_THROW(ExceptionDataIO, "Error parsing extra properties: ", error.what());
        }
    }

    SpacingInfo
    VesselSetFile::getOriginalSpacingInfo() const
    {
        if (m_extraProperties != nullptr)
        {
            return SpacingInfo::getDefaultForNVista3();
        }
        return SpacingInfo::getDefault();
    }

} // namespace isx
