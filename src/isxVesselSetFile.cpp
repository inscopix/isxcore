#include "isxVesselSetFile.h"
#include "isxImage.h"
#include "isxException.h"
#include "isxAssert.h"
#include "isxPathUtils.h"

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
                const SpacingInfo & inSpacingInfo,
                const VesselSetType_t inVesselSetType)
                : m_fileName(inFileName)
                , m_timingInfo(inTimingInfo)
                , m_spacingInfo(inSpacingInfo)
                , m_vesselSetType(inVesselSetType)
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

        const isize_t offsetInBytes = lineEndpointsSizeInBytes();
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
    VesselSetFile::readProjectionImage()
    {
        // projection image is the same for all vessels
        // it is stored before individual vessel data
        const isize_t pos = 0;
        m_file.seekg(pos, std::ios_base::beg);

        if (!m_file.good())
        {
            ISX_THROW(isx::ExceptionFileIO, "Error seeking to vessel projection image.");
        }

        SpImage_t image = std::make_shared<Image>(
            m_spacingInfo,
            sizeof(float) * m_spacingInfo.getNumColumns(),
            1,
            DataType::F32);
        m_file.read(image->getPixels(), image->getImageSizeInBytes());

        if (!m_file.good())
        {
            ISX_THROW(isx::ExceptionFileIO, "Error reading vessel projection image.");
        }

        return image;
    }

    SpVesselLine_t
    VesselSetFile::readLineEndpoints(isize_t inVesselId)
    {
        seekToVessel(inVesselId);

        if (!m_file.good())
        {
            ISX_THROW(isx::ExceptionFileIO, "Error seeking to vessel line endpoints for read.");
        }

        SpVesselLine_t lineEndpoints = std::make_shared<VesselLine>();
        const size_t numPoints = (m_vesselSetType == VesselSetType_t::RBC_VELOCITY) ? 4 : 2;
        for (size_t i = 0; i < numPoints; i++)
        {
            int64_t x, y;
            m_file.read(reinterpret_cast<char*>(&x), sizeof(int64_t));
            m_file.read(reinterpret_cast<char*>(&y), sizeof(int64_t));
            lineEndpoints->m_contour.push_back(isx::PointInPixels_t(x, y));
        }

        if (!m_file.good())
        {
            ISX_THROW(isx::ExceptionFileIO, "Error reading vessel line endpoints.");
        }

        return lineEndpoints;
    }

    SpFTrace_t
    VesselSetFile::readDirectionTrace(isize_t inVesselId)
    {
        if (m_vesselSetType != VesselSetType_t::RBC_VELOCITY)
        {
            ISX_THROW(isx::ExceptionUserInput, "Reading direction for diameter vessel set");
        }

        if (!m_directionSaved)
        {
            return nullptr;
        }

        seekToVessel(inVesselId);

        // Calculate bytes till beginning of vessel data
        const isize_t offsetInBytes = lineEndpointsSizeInBytes() + traceSizeInBytes();
        m_file.seekg(offsetInBytes, std::ios_base::cur);

        if (!m_file.good())
        {
            ISX_THROW(isx::ExceptionFileIO, "Error seeking to vessel direction trace for read.");
        }

        SpFTrace_t direction = std::make_shared<Trace<float>>(m_timingInfo);
        m_file.read(reinterpret_cast<char*>(direction->getValues()), directionSizeInBytes());

        if (!m_file.good())
        {
            ISX_THROW(isx::ExceptionFileIO, "Error reading vessel direction trace.");
        }

        return direction;
    }

    SpVesselCorrelations_t
    VesselSetFile::readCorrelations(isize_t inVesselId, isize_t inFrameNumber)
    {
        if (m_vesselSetType != VesselSetType_t::RBC_VELOCITY)
        {
            ISX_THROW(isx::ExceptionUserInput, "Correlation triptychs can only be read from rbc velocity set but this is a vessel diameter set");
        }

        if (!isCorrelationSaved())
        {
            return nullptr;
        }

        seekToVessel(inVesselId);

        // Calculate bytes till beginning of vessel data
        const isize_t correlationBytes = correlationSizeInBytes(inVesselId);
        const isize_t offsetInBytes = lineEndpointsSizeInBytes() + traceSizeInBytes() + directionSizeInBytes()
                                      + (correlationBytes * 3) * inFrameNumber;

        m_file.seekg(offsetInBytes, std::ios_base::cur);

        if (!m_file.good())
        {
            ISX_THROW(isx::ExceptionFileIO, "Error seeking to vessel correlation triptych for read.");
        }

        SpVesselCorrelations_t correlations = std::make_shared<VesselCorrelations>(m_correlationSizes[inVesselId]);
        for (int offset = -1; offset <= 1; offset++)
        {
            m_file.read(reinterpret_cast<char*>(correlations->getValues(offset)), correlationBytes);
        }
        if (!m_file.good())
        {
            ISX_THROW(isx::ExceptionFileIO, "Error reading vessel correlation triptych.");
        }
        return correlations;
    }

    SpFTrace_t
    VesselSetFile::readCenterTrace(const isize_t inVesselId)
    {
        if (m_vesselSetType != VesselSetType_t::VESSEL_DIAMETER)
        {
            ISX_THROW(isx::ExceptionUserInput, "Model center can only be read from vessel diameter set but this is a rbc velocity set");
        }

        if (!m_centerSaved)
        {
            return nullptr;
        }

        seekToVessel(inVesselId);

        // Calculate bytes till beginning of vessel data
        const isize_t offsetInBytes = lineEndpointsSizeInBytes() + traceSizeInBytes();
        m_file.seekg(offsetInBytes, std::ios_base::cur);

        if (!m_file.good())
        {
            ISX_THROW(isx::ExceptionFileIO, "Error seeking to vessel center trace for read.");
        }


        SpFTrace_t centerPointTrace = std::make_shared<Trace<float>>(m_timingInfo);
        m_file.read(reinterpret_cast<char*>(centerPointTrace->getValues()), traceSizeInBytes());

        if (!m_file.good())
        {
            ISX_THROW(isx::ExceptionFileIO, "Error reading vessel center trace.");
        }

        return centerPointTrace;
    }

    void
    VesselSetFile::writeImage(const SpImage_t & inProjectionImage)
    {
        if (m_fileClosedForWriting)
        {
            ISX_THROW(isx::ExceptionFileIO,
                      "Writing data after file was closed for writing.", m_fileName);
        }

        // ensure projection image is of type F32
        const DataType dataType = inProjectionImage->getDataType();
        if (dataType != DataType::F32)
        {
            ISX_THROW(isx::ExceptionDataIO,
                      "Expected F32 data type for the projection image, instead got: ", dataType);
        }

        // validate size of the projection image
        isize_t inImageSizeInBytes = inProjectionImage->getImageSizeInBytes();
        isize_t fImageSizeInBytes = projectionImageSizeInBytes();
        ISX_ASSERT(inImageSizeInBytes == fImageSizeInBytes);

        isize_t pos = 0;
        m_file.seekp(pos, std::ios_base::beg);
        m_file.write(inProjectionImage->getPixels(), inImageSizeInBytes);

        if (!m_file.good())
        {
            ISX_THROW(isx::ExceptionFileIO,
                      "Failed to write vessel data to file: ", m_fileName);
        }

        m_headerOffset = m_file.tellp();
        flush();
    }

    void
    VesselSetFile::prepareVesselForWrite(const isize_t inVesselId, const std::string & inName)
    {
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
    }

    void
    VesselSetFile::writeVesselDiameterData(
        const isize_t inVesselId,
        const SpVesselLine_t & inLineEndpoints,
        const SpFTrace_t & inDiameterTrace,
        const SpFTrace_t & inCenterTrace,
        const std::string & inName)
    {
        if (m_fileClosedForWriting)
        {
            ISX_THROW(isx::ExceptionFileIO,
                      "Writing data after file was closed for writing.", m_fileName);
        }

        ISX_ASSERT(inLineEndpoints->m_contour.size() == 2);
        ISX_ASSERT(inDiameterTrace->getTimingInfo().getNumTimes() == m_timingInfo.getNumTimes());

        m_centerSaved = true;

        prepareVesselForWrite(inVesselId, inName);

        // write line endpoints
        for (size_t i = 0; i < 2; i++)
        {
            const int64_t x = inLineEndpoints->m_contour[i].getX();
            const int64_t y = inLineEndpoints->m_contour[i].getY();
            m_file.write(reinterpret_cast<const char*>(&x), sizeof(int64_t));
            m_file.write(reinterpret_cast<const char*>(&y), sizeof(int64_t));
        }

        // write trace
        m_file.write(reinterpret_cast<char*>(inDiameterTrace->getValues()), traceSizeInBytes());

        // write center trace
        m_file.write(reinterpret_cast<char*>(inCenterTrace->getValues()), traceSizeInBytes());

        if (!m_file.good())
        {
            ISX_THROW(isx::ExceptionFileIO,
                      "Failed to write vessel data to file: ", m_fileName);
        }
        m_headerOffset = m_file.tellp();
        flush();
    }

    void
    VesselSetFile::writeVesselVelocityData(
        const isize_t inVesselId,
        const SpVesselLine_t & inLineEndpoints,
        const SpFTrace_t & inVelocityTrace,
        const SpFTrace_t & inDirectionTrace,
        const SpVesselCorrelationsTrace_t & inCorrTrace,
        const std::string & inName)
    {
        if (m_fileClosedForWriting)
        {
            ISX_THROW(isx::ExceptionFileIO,
                      "Writing data after file was closed for writing.", m_fileName);
        }

        ISX_ASSERT(inLineEndpoints->m_contour.size() == 4);
        ISX_ASSERT(inVelocityTrace->getTimingInfo().getNumTimes() == m_timingInfo.getNumTimes());
        ISX_ASSERT(inDirectionTrace->getTimingInfo().getNumTimes() == m_timingInfo.getNumTimes());

        if (inCorrTrace)
        {
            ISX_ASSERT(inCorrTrace->getTimingInfo().getNumTimes() == m_timingInfo.getNumTimes());

            if (inVesselId == m_numVessels)
            {
                m_correlationSizes.push_back(SizeInPixels_t());
            }
            ISX_ASSERT(m_correlationSizes.size() == m_numVessels + 1);

            const SpVesselCorrelations_t triptych  = inCorrTrace->getValue(0);
            m_correlationSizes[inVesselId] = triptych->getNumPixels();
        }
        
        // flag indicating if direction is saved to disk, for backwards compatibility
        m_directionSaved = true;

        prepareVesselForWrite(inVesselId, inName);

        // write line endpoints
        for (size_t i = 0; i < 4; i++)
        {
            const int64_t x = inLineEndpoints->m_contour[i].getX();
            const int64_t y = inLineEndpoints->m_contour[i].getY();
            m_file.write(reinterpret_cast<const char*>(&x), sizeof(int64_t));
            m_file.write(reinterpret_cast<const char*>(&y), sizeof(int64_t));
        }

        // write trace
        m_file.write(reinterpret_cast<char*>(inVelocityTrace->getValues()), traceSizeInBytes());

        // write direction trace
        m_file.write(reinterpret_cast<char*>(inDirectionTrace->getValues()), directionSizeInBytes());

        // write correlation trace
        if (inCorrTrace)
        {
            const size_t numSamples = inCorrTrace->getTimingInfo().getNumTimes();
            const size_t correlationSize = correlationSizeInBytes(inVesselId);
            for (size_t i = 0; i < numSamples; i++)
            {
                const SpVesselCorrelations_t triptych  = inCorrTrace->getValue(i);
                for (int offset = -1; offset <= 1; offset++)
                {
                    m_file.write(reinterpret_cast<char*>(triptych->getValues(offset)), correlationSize);
                }
            }
        }

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

            m_vesselSetType = (m_extraProperties["idps"]["vesselset"]["type"].get<std::string>() == "red blood cell velocity") ? 
                                    VesselSetType_t::RBC_VELOCITY : VesselSetType_t::VESSEL_DIAMETER;

            if (j.find("VesselDirectionSaved") != j.end())
            {
                m_directionSaved = true;
            }

            if (j.find("VesselCenterSaved") != j.end())
            {
                m_centerSaved = true;
            }

            if (j.find("efocusValues") != j.end())
            {
                m_efocusValues = j["efocusValues"].get<std::vector<uint16_t>>();
            }
            else if (m_extraProperties["idps"].find("efocus") != m_extraProperties["idps"].end())
            {
                m_efocusValues = {m_extraProperties["idps"]["efocus"].get<uint16_t>()};
            }

            if (j.find("VesselCorrelationSizes") != j.end())
            {
                for (auto & el : j["VesselCorrelationSizes"])
                {
                    m_correlationSizes.push_back(SizeInPixels_t(
                        el[0].get<size_t>(),
                        el[1].get<size_t>()
                    ));
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

        size_t vesselsSize = 0;
        for (size_t i = 0; i < m_vesselNames.size(); i++)
        {
            if (i == 0) vesselsSize += projectionImageSizeInBytes();
            vesselsSize += vesselDataSizeInBytes(i);
            if (i == m_vesselNames.size() - 1) vesselsSize += lineEndpointsSizeInBytes(); // see note in writeHeader()
        }

        if (size_t(m_headerOffset) != vesselsSize)
        {
            ISX_THROW(isx::ExceptionDataIO, "Number of vessels in header does not match number of vessels in file.");
        }
        m_numVessels = m_vesselNames.size();
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
            saveVesselSetType();
            j["extraProperties"] = m_extraProperties;
            j["efocusValues"] = m_efocusValues;
            if (isCorrelationSaved()) j["VesselCorrelationSizes"] = convertVesselSetCorrelationSizesToJson();
            if (m_directionSaved) j["VesselDirectionSaved"] = true;
            if (m_centerSaved) j["VesselCenterSaved"] = true;
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
            // Note: This overshoots the end of the vessel by lineEndpointsSizeInBytes()
            // It should actually be vesselSize = vesselDataSizeInBytes(m_numVessels - 1);
            // But it doesn't prevent the file from being read from and written to correctly
            // And by this point alpha testers have created vessel sets so leaving this in for backwards compatability
            seekToVessel(m_numVessels - 1);    
            isize_t vesselSize = lineEndpointsSizeInBytes() + vesselDataSizeInBytes(m_numVessels - 1);
            m_file.seekp(vesselSize, std::ios_base::cur);
        }

        m_headerOffset = m_file.tellp();
        writeJsonHeaderAtEnd(j, m_file);

        flush();
    }

    void
    VesselSetFile::seekToVessel(const isize_t inVesselId)
    {
        if (inVesselId >= m_numVessels)
        {
            ISX_THROW(isx::ExceptionFileIO,
                      "Unable to seek to vessel ID ", inVesselId, " in file: ", m_fileName);
        }

        const size_t pos = vesselOffsetInBytes(inVesselId);
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
    VesselSetFile::projectionImageSizeInBytes()
    {
        return m_spacingInfo.getTotalNumPixels() * sizeof(float);
    }

    isize_t
    VesselSetFile::lineEndpointsSizeInBytes()
    {
        // two or four (x,y) points where x and y are each unsigned 64-bit integers
        if (m_vesselSetType == VesselSetType_t::RBC_VELOCITY)
        {
            return 8 * sizeof(uint64_t);
        }
        else
        {
            return 4 * sizeof(uint64_t);
        }
    }

    isize_t
    VesselSetFile::traceSizeInBytes()
    {
        return m_timingInfo.getNumTimes() * sizeof(float);
    }

    isize_t
    VesselSetFile::directionSizeInBytes()
    {
        return m_timingInfo.getNumTimes() * sizeof(float);
    }

    isize_t
    VesselSetFile::correlationSizeInBytes(isize_t inVesselId)
    {
        return m_correlationSizes[inVesselId].getWidth() * m_correlationSizes[inVesselId].getHeight() * sizeof(float);
    }

    isize_t
    VesselSetFile::correlationTraceSizeInBytes(isize_t inVesselId)
    {
        return 3 * correlationSizeInBytes(inVesselId) * m_timingInfo.getNumTimes();
    }

    isize_t
    VesselSetFile::vesselDataSizeInBytes(isize_t inVesselId)
    {
        size_t size = lineEndpointsSizeInBytes() + traceSizeInBytes();
        if (m_vesselSetType == VesselSetType_t::RBC_VELOCITY)
        {
            if (m_directionSaved)
            {
                size += directionSizeInBytes();
            }
            if (isCorrelationSaved())
            {
                size += correlationTraceSizeInBytes(inVesselId);
            }
        }
        else if (m_centerSaved)
        {
            size += traceSizeInBytes();
        }
        return size;
    }

    isize_t
    VesselSetFile::vesselOffsetInBytes(isize_t inVesselId)
    {
        size_t offset = projectionImageSizeInBytes();
        for (size_t i = 0; i < inVesselId; i++)
        {
            offset += vesselDataSizeInBytes(i);        
        }
        return offset;
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
        const size_t width = calculateWidthOfPaddedName(m_vesselNames.size());
        for (isize_t i = 0; i < m_numVessels; ++i)
        {
            if (m_vesselNames[i].empty())
            {
                m_vesselNames[i] = createNumberPaddedName("V", i, width);
            }
        }
    }

    void
    VesselSetFile::saveVesselSetType()
    {
        m_extraProperties["idps"]["vesselset"]["type"] = getVesselSetTypeString(m_vesselSetType);
    }

    json
    VesselSetFile::convertVesselSetCorrelationSizesToJson()
    {
        ISX_ASSERT(m_correlationSizes.size() == m_numVessels);
        json j = json::array();
        for (size_t i = 0; i < m_numVessels; i++)
        {
            j.push_back({m_correlationSizes[i].getWidth(), m_correlationSizes[i].getHeight()});
        }
        return j;
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

    VesselSetType_t
    VesselSetFile::getVesselSetType() const
    {
        return m_vesselSetType;
    }

    SizeInPixels_t
    VesselSetFile::getCorrelationSize(isize_t inVesselId) const
    {
        return m_correlationSizes[inVesselId];
    }

    bool
    VesselSetFile::isCorrelationSaved() const
    {
        return !m_correlationSizes.empty();
    }

    bool
    VesselSetFile::isDirectionSaved() const
    {
        return m_directionSaved;
    }

    bool
    VesselSetFile::isCenterSaved() const
    {
        return m_centerSaved;
    }

} // namespace isx
