#include "isxNVista3GpioFile.h"
#include "isxPathUtils.h"
#include "isxJsonUtils.h"
#include "isxException.h"
#include <algorithm>
#include <cstring>

namespace isx
{

const std::map<NVista3GpioFile::Channel, std::string> NVista3GpioFile::s_channelNames
{
    {NVista3GpioFile::Channel::FRAME_COUNTER, "Frame Count"},
    {NVista3GpioFile::Channel::DIGITAL_GPI, "Digital GPI"},
    {NVista3GpioFile::Channel::BNC_GPIO_IN_1, "BNC GPIO IN 1"},
    {NVista3GpioFile::Channel::BNC_GPIO_IN_2, "BNC GPIO IN 2"},
    {NVista3GpioFile::Channel::BNC_GPIO_IN_3, "BNC GPIO IN 3"},
    {NVista3GpioFile::Channel::BNC_GPIO_IN_4, "BNC GPIO IN 4"},
    {NVista3GpioFile::Channel::EX_LED, "EX_LED"},
    {NVista3GpioFile::Channel::OG_LED, "OG_LED"},
    {NVista3GpioFile::Channel::DI_LED, "DI_LED"},
    {NVista3GpioFile::Channel::EFOCUS, "efocus"},
    {NVista3GpioFile::Channel::TRIG_SYNC_FLASH, "TRIG SYNC Flash"},
};

NVista3GpioFile::NVista3GpioFile()
{
}

NVista3GpioFile::NVista3GpioFile(const std::string & inFileName, const std::string & inOutputDir)
    : m_fileName(inFileName)
    , m_outputDir(inOutputDir)
{
    m_file.open(m_fileName, std::ios::binary | std::ios_base::in);
    if (!m_file.good() || !m_file.is_open())
    {
        ISX_THROW(ExceptionFileIO, "Failed to open GPIO data file for reading: ", m_fileName);
    }
    m_valid = true;
}

NVista3GpioFile::~NVista3GpioFile()
{
    isx::closeFileStreamWithChecks(m_file, m_fileName);
}

bool
NVista3GpioFile::isValid()
{
    return m_valid;
}

const std::string &
NVista3GpioFile::getFileName()
{
    return m_fileName;
}

void
NVista3GpioFile::setCheckInCallback(AsyncCheckInCB_t inCheckInCB)
{
    m_checkInCB = inCheckInCB;
}

void
NVista3GpioFile::skip(const size_t inNumBytes)
{
    m_file.ignore(inNumBytes);
}

AsyncTaskStatus
NVista3GpioFile::parse()
{
    bool foundSync = false;

    uint32_t sync;
    PktHeader header;

//    uint64_t tsc;
//    uint32_t fc;
    //uint32_t digitalGpi;
    //uint16_t bncGpioIn1, bncGpioIn2, bncGpioIn3, bncGpioIn4;
    //uint16_t exLed, ogLed, diLed;
    //uint16_t eFocus;
    //uint16_t trigSyncFlash;
//    uint16_t bncTrig, bncSync;

    std::map<Channel, std::vector<EventBasedFileV2::DataPkt>> packets;

    while (!m_file.eof())
    {
        read(sync);
        if (sync != s_syncWord)
        {
            continue;
        }
        foundSync = true;
        ISX_LOG_DEBUG("Found sync");

        read(header);
        ISX_LOG_DEBUG("Read packet type ", header.m_type);
        if ((header.m_type >> 8) != s_eventSignature)
        {
            ISX_LOG_DEBUG("Found non-event header");
            continue;
        }
        ISX_LOG_DEBUG("Read sequence ", header.m_sequence);
        ISX_LOG_DEBUG("Read payloadSize ", header.m_payloadSize);

        //if (Event(header.m_type) == Event::WAVEFORM)
        //{
        //    ISX_LOG_DEBUG("Waveform");
        //    skip(4);
        //    continue;
        //}

//        read(tsc);
//        ISX_LOG_DEBUG("Read TSC ", tsc);
//        read(fc);
//        ISX_LOG_DEBUG("Read FC ", fc);

        std::vector<uint32_t> payload(header.m_payloadSize);
        m_file.read(reinterpret_cast<char *>(payload.data()), header.m_payloadSize * sizeof(uint32_t));

//        switch (Event(header.m_type))
//        {
//            case Event::CAPTURE_ALL:
//                read(digitalGpi);
//                read(bncGpioIn1);
//                read(bncGpioIn2);
//                read(bncGpioIn3);
//                read(bncGpioIn4);
//                read(exLed);
//                read(ogLed);
//                read(diLed);
//                skip(2);
//                read(eFocus);
//                skip(2);
//                read(trigSyncFlash);
//                break;
//
//            case Event::CAPTURE_GPIO:
//                read(digitalGpi);
//                read(bncTrig);
//                read(bncGpioIn1);
//                read(bncGpioIn2);
//                read(bncGpioIn3);
//                read(bncGpioIn4);
//                break;
//
//            case Event::BNC_GPIO_IN_1:
//                read(bncGpioIn1);
//                skip(2);
//                break;
//
//            case Event::BNC_GPIO_IN_2:
//                read(bncGpioIn2);
//                skip(2);
//                break;
//
//            case Event::BNC_GPIO_IN_3:
//                read(bncGpioIn3);
//                skip(2);
//                break;
//
//            case Event::BNC_GPIO_IN_4:
//                read(bncGpioIn4);
//                skip(2);
//                break;
//
//            case Event::DIGITAL_GPI:
//                read(digitalGpi);
//                skip(2);
//                break;
//
//            case Event::EX_LED:
//                read(exLed);
//                skip(2);
//                break;
//
//            case Event::OG_LED:
//                read(ogLed);
//                skip(2);
//                break;
//
//            case Event::DI_LED:
//                read(diLed);
//                skip(2);
//                break;
//
//            case Event::FRAME_COUNT:
//                break;
//
//            case Event::BNC_TRIG:
//                read(bncTrig);
//                skip(2);
//                break;
//
//            case Event::BNC_SYNC:
//                read(bncSync);
//                skip(2);
//                break;
//
//            case Event::WAVEFORM:
//                ISX_ASSERT("Help me god");
//                break;
//
//            default:
//                break;
//        }

    }

    if (!foundSync)
    {
        ISX_THROW(ExceptionFileIO, "Failed to find the beginning of the data stream and parse the file.");
    }

    return isx::AsyncTaskStatus::COMPLETE;
}

const std::string &
NVista3GpioFile::getOutputFileName() const
{
    return m_outputFileName;
}

} // namespace isx
