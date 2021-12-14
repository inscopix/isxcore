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
        seekToVesselForRead(inVesselId, true);

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
        seekToProjectionImageForRead();

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
        seekToVesselForRead(inVesselId, false);

        if (!m_file.good())
        {
            ISX_THROW(isx::ExceptionFileIO, "Error seeking to vessel line endpoints for read.");
        }

        SpVesselLine_t lineEndpoints = std::make_shared<VesselLine>();
        size_t numPoints = (m_vesselSetType == VesselSetType_t::RBC_VELOCITY) ? 4 : 2;
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

        seekToVesselForRead(inVesselId, true);

        // Calculate bytes till beginning of vessel data
        isize_t offsetInBytes = traceSizeInBytes();
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
            ISX_THROW(isx::ExceptionUserInput, "Reading correlation triptychs for diameter vessel set");
        }

        if (!correlationSaved())
        {
            return nullptr;
        }

        seekToVesselForRead(inVesselId, true);

        // Calculate bytes till beginning of vessel data
        isize_t offsetInBytes = traceSizeInBytes() + directionSizeInBytes();
        isize_t correlationBytes = correlationSizeInBytes(inVesselId);
        offsetInBytes += (correlationBytes * 3) * inFrameNumber;

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

    void
    VesselSetFile::writeVesselData(isize_t inVesselId, const Image & inProjectionImage, const SpVesselLine_t & inLineEndpoints,
                                   Trace<float> & inData, const std::string & inName, const SpFTrace_t & inDirectionTrace,
                                   const SpVesselCorrelationsTrace_t & inCorrTrace)
    {
        if (m_fileClosedForWriting)
        {
            ISX_THROW(isx::ExceptionFileIO,
                      "Writing data after file was closed for writing.", m_fileName);
        }

        // ensure projection image is of type F32
        const DataType dataType = inProjectionImage.getDataType();
        if (dataType != DataType::F32)
        {
            ISX_THROW(isx::ExceptionDataIO,
                      "Expected F32 data type for the projection image, instead got: ", dataType);
        }

        if (inCorrTrace)
        {
            if (inVesselId == m_numVessels)
            {
                m_correlationSizes.push_back(SizeInPixels_t());
            }
            ISX_ASSERT(m_correlationSizes.size() == m_numVessels + 1);

            SpVesselCorrelations_t triptych  = inCorrTrace->getValue(0);
            m_correlationSizes[inVesselId] = triptych->getNumPixels();
        }
        
        if (inDirectionTrace && !m_directionSaved)
        {
            m_directionSaved = true;
        }

        // validate size of the projection image
        isize_t inImageSizeInBytes = inProjectionImage.getImageSizeInBytes();
        isize_t fImageSizeInBytes = projectionImageSizeInBytes();
        ISX_ASSERT(inImageSizeInBytes == fImageSizeInBytes);

        // validate trace length
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
            seekToVesselForWrite(inVesselId);
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

        // write projection image only if writing first vessel, otherwise skip ahead
        if (inVesselId == 0)
        {
            m_file.write(inProjectionImage.getPixels(), inImageSizeInBytes);
        }

        // write line endpoints
        if (m_vesselSetType == VesselSetType_t::RBC_VELOCITY)
        {
            ISX_ASSERT(inLineEndpoints->m_contour.size() == 4);
        }
        else
        {
            ISX_ASSERT(inLineEndpoints->m_contour.size() == 2);
        }
        for (size_t i = 0; i < inLineEndpoints->m_contour.size(); i++)
        {
            int64_t x = inLineEndpoints->m_contour[i].getX();
            int64_t y = inLineEndpoints->m_contour[i].getY();
            m_file.write(reinterpret_cast<const char*>(&x), sizeof(int64_t));
            m_file.write(reinterpret_cast<const char*>(&y), sizeof(int64_t));
        }

        // write trace
        m_file.write(reinterpret_cast<char*>(inData.getValues()), traceSizeInBytes());

        if (m_vesselSetType == VesselSetType_t::RBC_VELOCITY)
        {
            ISX_ASSERT(inDirectionTrace != nullptr);
            m_file.write(reinterpret_cast<char*>(inDirectionTrace->getValues()), directionSizeInBytes());

            if (inCorrTrace)
            {
                size_t numSamples = inCorrTrace->getTimingInfo().getNumTimes();
                size_t correlationSize = correlationSizeInBytes(inVesselId);
                for (size_t i = 0; i < numSamples; i++)
                {
                    SpVesselCorrelations_t triptych  = inCorrTrace->getValue(i);
                    for (int offset = -1; offset <= 1; offset++)
                    {
                        m_file.write(reinterpret_cast<char*>(triptych->getValues(offset)), correlationSize);
                    }
                }
            }
        }
        else
        {
            ISX_ASSERT(inDirectionTrace == nullptr);
            ISX_ASSERT(inCorrTrace == nullptr);
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
            if (correlationSaved()) j["VesselCorrelationSizes"] = convertVesselSetCorrelationSizesToJson();
            if (m_directionSaved) j["VesselDirectionSaved"] = true;
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
            // It should actually be seekToVesselForRead(m_numVessels - 1, false);
            // But it doesn't prevent the file from being read from and written to correctly
            // And by this point alpha testers have created vessel sets so leaving this in for backwards compatability
            seekToVesselForRead(m_numVessels - 1, true);
            isize_t vesselSize = vesselDataSizeInBytes(m_numVessels - 1);
            m_file.seekp(vesselSize, std::ios_base::cur);
        }

        m_headerOffset = m_file.tellp();
        writeJsonHeaderAtEnd(j, m_file);

        flush();
    }

    void
    VesselSetFile::seekToProjectionImageForRead()
    {
        // projection image is the same for all vessels
        // it is stored before individual vessel data
        isize_t pos = 0;
        m_file.seekg(pos, std::ios_base::beg);

        if (!m_file.good() || pos >= isize_t(m_headerOffset))
        {
            ISX_THROW(isx::ExceptionFileIO, "Error reading projection image.");
        }

        if (m_openmode & std::ios_base::out)
        {
            m_file.seekp(m_file.tellg(), std::ios_base::beg);   // Make sure write and read pointers are tied together
        }
    }

    void
    VesselSetFile::seekToVesselForRead(isize_t inVesselId, const bool readTrace)
    {
        isize_t pos = 0;
        if (inVesselId >= m_numVessels)
        {
            ISX_THROW(isx::ExceptionFileIO,
                      "Unable to seek to vessel ID ", inVesselId, " in file: ", m_fileName);
        }

        pos = vesselOffsetInBytes(inVesselId);
        if (readTrace)
        {
            pos += lineEndpointsSizeInBytes();
        }

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

    void
    VesselSetFile::seekToVesselForWrite(isize_t inVesselId)
    {
        isize_t pos = 0;
        if (inVesselId >= m_numVessels)
        {
            ISX_THROW(isx::ExceptionFileIO,
                      "Unable to seek to vessel ID ", inVesselId, " in file: ", m_fileName);
        }

        pos = vesselOffsetInBytes(inVesselId);

        // update start position
        // if not writing the first vessel, skip the projection image
        if (inVesselId == 0)
        {
            pos -= projectionImageSizeInBytes();
        }

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
            if (correlationSaved())
            {
                size += correlationTraceSizeInBytes(inVesselId);
            }
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
    VesselSetFile::correlationSaved() const
    {
        return !m_correlationSizes.empty();
    }

} // namespace isx
