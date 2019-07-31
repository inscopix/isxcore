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
        ISX_THROW(ExceptionFileIO, "Failed to open IMU data file for reading: ", m_fileName);
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
    auto accAvgStep = DurationInSeconds::fromMilliseconds(s_accOriRate);
    auto magAvgStep = accAvgStep * 20;
    Time start;
    Time end;
    try
    {
        m_file.clear();
        m_file.seekg(header.sessionOffset, std::fstream::beg);
        json sessionFooter;
        m_file >> sessionFooter;
        m_sessionStr = sessionFooter.dump();

        // Read timing info
        json timingInfo = sessionFooter.at("timingInfo");
        start = convertJsonToTime(timingInfo.at("start"));
        end = convertJsonToTime(timingInfo.at("end"));
        auto accTi = timingInfo.find("accelerometer");
        if (accTi != timingInfo.end())
        {
            Ratio milliSecRatio = convertJsonToRatio(accTi.value().at("periodMs"));
            Ratio secRatio(milliSecRatio.getNum(), milliSecRatio.getDen() * 1000);
            accAvgStep = secRatio;
        }

        auto magTi = timingInfo.find("magnetometer");
        if (magTi != timingInfo.end())
        {
            Ratio milliSecRatio = convertJsonToRatio(magTi.value().at("periodMs"));
            Ratio secRatio(milliSecRatio.getNum(), milliSecRatio.getDen() * 1000);
            magAvgStep = secRatio;
        }
    }
    catch (const std::exception & inError)
    {
        ISX_LOG_WARNING("Failed to read json footer from IMU file with error: ", inError.what());
    }

    /// Write to isxd file
    const DurationInSeconds headerTime(header.epochTimeSecNum, header.epochTimeSecDen);
    const Time startTime(headerTime, header.utcOffset);
    const TimingInfo timing(
        startTime,
        accAvgStep,
        std::max({header.accCount, header.oriCount}));
    std::vector<DurationInSeconds> steps;
    steps.insert(steps.end(), s_maxAccArrSize + s_maxOriArrSize, accAvgStep); // acc + ori
    steps.insert(steps.end(), s_maxMagArrSize, magAvgStep); // mag
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
                for (isize_t j = 0; j < header.accCount; ++j)
                {
                    EventBasedFileV2::DataPkt pkt;
                    pkt.offsetMicroSecs = (accPkts[j].timeStamp - accPkts[0].timeStamp) * 1000;
                    pkt.value = accPkts[j].accData[i];
                    pkt.signal = i;
                    outputFile.writeDataPkt(pkt);
                }
                break;
            }
            case IMUSignalType::MAG:
            {
                for (isize_t j = 0; j < header.magCount; ++j)
                {
                    EventBasedFileV2::DataPkt pkt;
                    pkt.offsetMicroSecs = (magPkts[j].timeStamp - accPkts[0].timeStamp) * 1000; // use acc as it is earliest
                    pkt.value = magPkts[j].magData[i-s_maxAccArrSize];
                    pkt.signal = i;
                    outputFile.writeDataPkt(pkt);
                }
                break;
            }
            case IMUSignalType::ORI:
            {
                for (isize_t j = 0; j < header.oriCount; ++j)
                {
                    EventBasedFileV2::DataPkt pkt;
                    pkt.offsetMicroSecs = (oriPkts[j].timeStamp - accPkts[0].timeStamp) * 1000; // use acc as it is earliest
                    pkt.value = float(oriPkts[j].oriData[i-s_maxAccArrSize-s_maxMagArrSize]) / float(2048); // S4.11
                    pkt.signal = i;
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
