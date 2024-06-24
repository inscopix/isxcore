#include "isxIMUFile.h"

#include "isxAsync.h"
#include "isxException.h"
#include "isxGpioUtils.h"
#include "isxJsonUtils.h"
#include "isxPathUtils.h"

namespace isx
{

IMUFile::IMUFile(const std::string & inFileName, const std::string & inOutputDir)
    : m_fileName(inFileName)
    , m_outputDir(inOutputDir)
{
    m_file.open(m_fileName, std::ios::binary | std::ios_base::in);
    if (!m_file.good() || !m_file.is_open())
    {
        ISX_THROW(ExceptionFileIO, "Failed to open IMU data file for reading (", m_fileName, ")", " with error: ", getSystemErrorString());
    }

    m_channels.push_back("Acc x");
    m_channels.push_back("Acc y");
    m_channels.push_back("Acc z");

    m_channels.push_back("Ori yaw");
    m_channels.push_back("Ori pitch");
    m_channels.push_back("Ori roll");

    m_channels.push_back("Mag x");
    m_channels.push_back("Mag y");
    m_channels.push_back("Mag z");
}

IMUFile::~IMUFile()
{
    isx::closeFileStreamWithChecks(m_file, m_fileName);
}

const std::string &
IMUFile::getFileName()
{
    return m_fileName;
}

const std::string &
IMUFile::getOutputFileName() const
{
    return m_outputFileName;
}

void
IMUFile::setCheckInCallback(AsyncCheckInCB_t inCheckInCB)
{
    m_checkInCB = inCheckInCB;
}

AsyncTaskStatus
IMUFile::parse()
{
    m_outputFileName = m_outputDir + "/" + isx::getBaseName(m_fileName) + "_imu.isxd";

    // TODO: Catch dropped packets

    /// Read imu file
    IMUHeader header{};
    m_file.seekg(0, std::fstream::beg);
    m_file.read((char *)&header, sizeof(header));
    ISX_LOG_DEBUG("header read");

    size_t progress = 0;
    m_file.seekg(0, std::fstream::end);
    const double progressMultiplier = 100.0 / double(m_file.tellg());

    // This is kept separate here in case payload size/field differs from each other in the future
    // accelerometer
    auto accPkts = new AccPayload[header.accCount];
    m_file.seekg(header.accOffset, std::fstream::beg);
    ISX_ASSERT(header.accSize == sizeof(AccPayload) * header.accCount);
    for (isize_t i = 0; i < header.accCount; ++i)
    {
        AccPayload accPkt{};
        m_file.read((char *)&accPkt, sizeof(accPkt));
        accPkts[i] = accPkt;
        // Report the progress for every 100 packet
        if ((i % 100) == 0)
        {
            const auto newProgress = size_t(progressMultiplier * m_file.tellg());
            if (newProgress != progress)
            {
                progress = newProgress;
                if (m_checkInCB && m_checkInCB(float(progress / 100.0)))
                {
                    return AsyncTaskStatus::CANCELLED;
                }
            }
        }
    }
    ISX_LOG_DEBUG("All acc read: ", header.accCount);

    // magnetometer
    auto magPkts = new MagPayload[header.magCount];
    m_file.seekg(header.magOffset, std::fstream::beg);
    ISX_ASSERT(header.magSize == sizeof(MagPayload) * header.magCount);
    for (isize_t i = 0; i < header.magCount; ++i)
    {
        MagPayload magPkt{};
        m_file.read((char *) &magPkt, sizeof(magPkt));
        magPkts[i] = magPkt;

        // Report the progress for every 100 packet
        if ((i % 100) == 0)
        {
            const auto newProgress = size_t(progressMultiplier * m_file.tellg());
            if (newProgress != progress)
            {
                progress = newProgress;
                if (m_checkInCB && m_checkInCB(float(progress / 100.0)))
                {
                    return AsyncTaskStatus::CANCELLED;
                }
            }
        }
    }
    ISX_LOG_DEBUG("All mag read: ", header.magCount);

    // orientation
    auto oriPkts = new OriPayload[header.oriCount];
    m_file.seekg(header.oriOffset, std::fstream::beg);
    ISX_ASSERT(header.oriSize == sizeof(OriPayload) * header.oriCount);
    for (isize_t i = 0; i < header.oriCount; ++i)
    {
        OriPayload oriPkt{};
        m_file.read((char *)&oriPkt, sizeof(oriPkt));
        oriPkts[i] = oriPkt;
        // step for ori is not calculated as it suppose to be synced with acc

        // Report the progress for every 100 packet
        if ((i % 100) == 0)
        {
            const auto newProgress = size_t(progressMultiplier * m_file.tellg());
            if (newProgress != progress)
            {
                progress = newProgress;
                if (m_checkInCB && m_checkInCB(float(progress / 100.0)))
                {
                    return AsyncTaskStatus::CANCELLED;
                }
            }
        }
    }
    ISX_LOG_DEBUG("All ori read: ", header.oriCount);

    // Session footer
    TimingInfo accOriTimingInfo;
    TimingInfo magTimingInfo;
    Time start;
    Time end;
    try
    {
        m_file.clear();
        m_file.seekg(header.sessionOffset, std::fstream::beg);
        json sessionFooter;
        m_file >> sessionFooter;
        m_sessionStr = sessionFooter.dump();


        // Read timing info - start/end
        json timingInfo = sessionFooter.at("timingInfo");
        start = convertJsonToTime(timingInfo.at("start"));
        end = convertJsonToTime(timingInfo.at("end"));

        // Read timing info - acc/ori
        std::vector<isize_t> accDropped;
        auto accStep = DurationInSeconds::fromMilliseconds(s_accOriRate);
        isize_t accCount = 0; // normal + dropped, differs from header(only normal)
        auto accJsonTiI = timingInfo.find("accelerometer");
        if (accJsonTiI != timingInfo.end())
        {
            json accJsonTi = accJsonTiI.value();
            Ratio milliSecRatio = convertJsonToRatio(accJsonTi.at("periodMs"));
            Ratio secRatio(milliSecRatio.getNum(), milliSecRatio.getDen() * 1000);
            accStep = secRatio;
            accDropped = accJsonTi.at("dropped").get<std::vector<isize_t>>();
            accCount = accJsonTi.at("numTimes");
        }

        // Read timing info - mag
        std::vector<isize_t> magDropped;
        auto magStep = accStep * 20;
        isize_t magCount = 0; // normal + dropped, differs from header(only normal)
        auto magJsonTiI = timingInfo.find("magnetometer");
        if (magJsonTiI != timingInfo.end())
        {
            json magJsonTi = magJsonTiI.value();
            Ratio milliSecRatio = convertJsonToRatio(magJsonTi.at("periodMs"));
            Ratio secRatio(milliSecRatio.getNum(), milliSecRatio.getDen() * 1000);
            magStep = secRatio;
            magDropped = magJsonTi.at("dropped").get<std::vector<isize_t>>();
            magCount = magJsonTi.at("numTimes");
        }

        accOriTimingInfo = TimingInfo(start, accStep, accCount, accDropped);
        magTimingInfo = TimingInfo(start, magStep, magCount, magDropped);
    }
    catch (const std::exception & inError)
    {
        ISX_LOG_WARNING("Failed to read json footer from IMU file with error: ", inError.what());
    }

    /// Write to isxd file
    std::vector<DurationInSeconds> steps;
    steps.insert(steps.end(), s_maxAccArrSize + s_maxOriArrSize, accOriTimingInfo.getStep()); // acc + ori
    steps.insert(steps.end(), s_maxMagArrSize, magTimingInfo.getStep()); // mag
    std::vector<SignalType> types(m_channels.size(), SignalType::DENSE);

    EventBasedFileV2 outputFile(m_outputFileName, DataSet::Type::IMU, m_channels, steps, types);

    std::vector<IMUSignalType> channelList;
    channelList.insert(channelList.end(), s_maxAccArrSize, IMUSignalType::ACC);
    channelList.insert(channelList.end(), s_maxOriArrSize, IMUSignalType::ORI);
    channelList.insert(channelList.end(), s_maxMagArrSize, IMUSignalType::MAG);

    for (isize_t i = 0; i < channelList.size(); ++i)
    {
        switch (channelList[i])
        {
            case IMUSignalType::ACC:
            {
                for (isize_t j = 0; j < accOriTimingInfo.getNumTimes(); ++j)
                {
                    EventBasedFileV2::DataPkt pkt;
                    pkt.signal = i;
                    pkt.offsetMicroSecs =
                        accOriTimingInfo.convertIndexToStartTime(j).getSecsSinceEpoch().toMicroseconds()
                            -
                                start.getSecsSinceEpoch().toMicroseconds();
                    if (!accOriTimingInfo.isDropped(j))
                    {
                        pkt.value = accPkts[accOriTimingInfo.timeIdxToRecordedIdx(j)].accData[i];
                    }
                    else
                    {
                        pkt.value = std::numeric_limits<float>::quiet_NaN();
                    }
                    outputFile.writeDataPkt(pkt);
                }
                break;
            }
            case IMUSignalType::ORI:
            {
                for (isize_t j = 0; j < accOriTimingInfo.getNumTimes(); ++j)
                {
                    EventBasedFileV2::DataPkt pkt;
                    pkt.signal = i;
                    pkt.offsetMicroSecs =
                        accOriTimingInfo.convertIndexToStartTime(j).getSecsSinceEpoch().toMicroseconds()
                            -
                                start.getSecsSinceEpoch().toMicroseconds();
                    if (!accOriTimingInfo.isDropped(j))
                    {
                        pkt.value =
                            float(oriPkts[accOriTimingInfo.timeIdxToRecordedIdx(j)].oriData[i-s_maxAccArrSize])
                            / float(2048); // S4.11
                    }
                    else
                    {
                        pkt.value = std::numeric_limits<float>::quiet_NaN();
                    }
                    outputFile.writeDataPkt(pkt);
                }
                break;
            }
            case IMUSignalType::MAG:
            {
                for (isize_t j = 0; j < magTimingInfo.getNumTimes(); ++j)
                {
                    EventBasedFileV2::DataPkt pkt;
                    pkt.signal = i;
                    pkt.offsetMicroSecs =
                        magTimingInfo.convertIndexToStartTime(j).getSecsSinceEpoch().toMicroseconds()
                            -
                                start.getSecsSinceEpoch().toMicroseconds();

                    if (!magTimingInfo.isDropped(j))
                    {
                        pkt.value = magPkts[magTimingInfo.timeIdxToRecordedIdx(j)].magData[i-s_maxAccArrSize-s_maxOriArrSize];
                    }
                    else
                    {
                        pkt.value = std::numeric_limits<float>::quiet_NaN();
                    }
                    outputFile.writeDataPkt(pkt);
                }
                break;
            }
        }
    }

    delete[] accPkts;
    delete[] magPkts;
    delete[] oriPkts;

    outputFile.setExtraProperties(m_sessionStr);
    outputFile.setSmallStep(true);
    outputFile.setTimingInfo(start, end);
    outputFile.closeFileForWriting();

    return isx::AsyncTaskStatus::COMPLETE;
}

}
