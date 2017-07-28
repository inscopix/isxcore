#include "isxNVistaGpioFile.h"
#include "isxHdf5Utils.h"
#include "isxPathUtils.h"
#include "isxException.h"

#include <numeric>
#include <cmath>
#include <algorithm>

namespace isx 
{
    const int NUM_COLS_ANC_DATA = 15;

    NVistaGpioFile::NVistaGpioFile()
    {

    }

    NVistaGpioFile::NVistaGpioFile(const std::string & inFileName, const std::string & inOutputDir, AsyncCheckInCB_t inCheckInCB) :
        m_fileName(inFileName),
        m_outputDir(inOutputDir),
        m_checkInCB(inCheckInCB)
    {
        SpH5File_t file = std::make_shared<H5::H5File>(inFileName.c_str(), H5F_ACC_RDONLY);
        m_inputHandle = std::make_shared<Hdf5FileHandle>(file, H5F_ACC_RDONLY);
        initTimestamps();
        m_valid = true;
    }

    NVistaGpioFile::~NVistaGpioFile()
    {

    }

    bool 
    NVistaGpioFile::isValid()
    {
        return m_valid;
    }

    const std::string & 
    NVistaGpioFile::getFileName()
    {
        return m_fileName;
    }

    AsyncTaskStatus 
    NVistaGpioFile::parse()
    {
        float progress = 0.05f;
        isize_t numSamples = m_timestamps.size();
        std::vector<uint16_t> data(numSamples*NUM_COLS_ANC_DATA);
        m_counts.resize(numSamples);

        // Read anc_data and anc_data_count
        try
        {
            H5::DataSet ancData = m_inputHandle->get()->openDataSet("anc_data");
            H5::DataSet ancDataCount = m_inputHandle->get()->openDataSet("anc_data_count");

            ancData.read(data.data(), ancData.getDataType());

            /// Note: Sometimes the ancillary data count is saved as a 32-bit integer array and sometimes as 
            /// unsigned char. 
            H5::DataType type = ancDataCount.getDataType();

            switch (type.getSize())
            {
            case 1:
            {
                ancDataCount.read(m_counts.data(), ancDataCount.getDataType());
                break;
            }
            case 4:
            {
                std::vector<int32_t> count(numSamples);
                ancDataCount.read(count.data(), ancDataCount.getDataType());
                for (isize_t i(0); i < numSamples; ++i)
                {
                    m_counts[i] = uint8_t(count[i]);
                }
                break;
            }

            }
                            
        }
        catch (const H5::FileIException& error)
        {
            ISX_THROW(isx::ExceptionFileIO,
                "Failure caused by H5File operations.\n", error.getDetailMsg());
        }

        catch (const H5::DataSetIException& error)
        {
            ISX_THROW(isx::ExceptionDataIO,
                "Failure caused by DataSet operations.\n", error.getDetailMsg());
        }

        catch (...)
        {
            ISX_ASSERT(false, "Unhandled exception.");
        }

        if(m_checkInCB)
        {
            if(m_checkInCB(progress))
            {
                return isx::AsyncTaskStatus::CANCELLED;
            }
        }

        // Separate channel data
        m_signals.resize(4);
        std::vector<uint16_t> masks{0x000F, 0x00F0, 0x0F00, 0xF000};

        for (isize_t s(0); s < numSamples; ++s)
        {
            progress = 0.05f + 0.45f* (float(s)/float(numSamples));

            if(m_checkInCB)
            {
                if(m_checkInCB(progress))
                {
                    return isx::AsyncTaskStatus::CANCELLED;
                }
            }

            isize_t firstSample = s * NUM_COLS_ANC_DATA;

            for (isize_t c(0); c < m_counts.at(s); ++c)
            {
                uint16_t val = data.at(firstSample + c);

                for (isize_t i(0); i < 4; ++i)
                {
                    uint16_t bitval = (val & masks[i]) >> 4*i;
                    m_signals[0].push_back((bitval & 0x0001) > 0);
                    m_signals[1].push_back((bitval & 0x0002) > 0);
                    m_signals[2].push_back((bitval & 0x0004) > 0);
                    m_signals[3].push_back((bitval & 0x0008) > 0);
                }
            }
        }

        return writeLogicalFile();
    }

    AsyncTaskStatus
    NVistaGpioFile::writeLogicalFile()
    {
        // Get filename for output
        std::string outputFileName = m_outputDir + "/" + isx::getBaseName(m_fileName) + ".isxd";
        TimeStampedDataFile file(outputFileName, TimeStampedDataFile::StoredData::GPIO, true);

        // Set timing info
        isize_t numSamples = m_signals.front().size();
        Time start(DurationInSeconds(isize_t(m_timestamps.front() * 1E6), isize_t(1E6)));

        DurationInSeconds step = isx::DurationInSeconds(isize_t(1E3), isize_t(1E6));
        TimingInfo ti(start, step, numSamples);
        file.setTimingInfo(ti);

        float progress = 0.5f;
        bool cancelled = false;

        // Write headers and data
        std::vector<std::string> channelNames{"IO1", "IO2", "sync", "trigger"};

        for (isize_t i(0); i < m_signals.size(); ++i)
        {
            file.writeChannelHeader(channelNames[i], "", "", m_signals[i].size());

            double ts = m_timestamps[0];

            for (isize_t j(0); (j < m_signals[i].size() && !cancelled); ++j)
            {
                progress = 0.5f + 0.5f * (float(i) + float(j)/float(m_signals[i].size()))/ float(m_signals.size());

                if(m_checkInCB)
                {
                   cancelled = m_checkInCB(progress);
                }

                uint64_t timestamp = uint64_t(ts * 1E6 + 1000.0 * j);
                bool state = m_signals[i].at(j);
                float val = 0.f;
                if (state)
                {
                    val = 1.f;
                }
                TimeStampedDataFile::DataPkt pkt(timestamp, state, val);
                file.writeDataPkt(pkt);

            }
        }

        // Close file
        file.closeFileForWriting();

        if (cancelled)
        {
            std::remove(outputFileName.c_str());
            return isx::AsyncTaskStatus::CANCELLED;
        }

        m_outputFileNames.push_back(outputFileName);
        return isx::AsyncTaskStatus::COMPLETE;
    }

    void 
    NVistaGpioFile::getOutputFileNames(std::vector<std::string> & outFileNames)
    {
        outFileNames = m_outputFileNames;
    }

    void 
    NVistaGpioFile::initTimestamps()
    {
        std::vector<std::string> names;
        isx::internal::getHdf5ObjNames(m_fileName, "/", names);

        std::vector<std::string> potentialNames{"timeStamp", "timestamp"};
        std::string name;
        for (auto & n : potentialNames)
        {
            auto it = std::find(names.begin(), names.end(), n);
            if (it != names.end())
            {
                name = n;
                break;
            }
        }
        
        if (!name.empty())
        {   
            std::vector<hsize_t> timestampsDims;
            std::vector<hsize_t> timestampsMaxDims;           

            try
            {
                H5::DataSet timeStampsDataSet = m_inputHandle->get()->openDataSet(name);
                isx::internal::getHdf5SpaceDims(timeStampsDataSet.getSpace(), timestampsDims, timestampsMaxDims);

                hsize_t numSamples = timestampsDims[0];
                m_timestamps.resize(numSamples);

                timeStampsDataSet.read(m_timestamps.data(), timeStampsDataSet.getDataType());                
            }
            catch (const H5::FileIException& error)
            {
                ISX_THROW(isx::ExceptionFileIO,
                    "Failure caused by H5File operations.\n", error.getDetailMsg());
            }

            catch (const H5::DataSetIException& error)
            {
                ISX_THROW(isx::ExceptionDataIO,
                    "Failure caused by DataSet operations.\n", error.getDetailMsg());
            }

            catch (...)
            {
                ISX_ASSERT(false, "Unhandled exception.");
            }
        }
        else
        {
            ISX_THROW(isx::ExceptionDataIO,
                "The file does not contain any GPIO data. Timestamps could not be read.\n");
        }

    }
}