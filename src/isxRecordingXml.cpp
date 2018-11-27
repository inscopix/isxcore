#include "isxRecordingXml.h"
#include "isxException.h"
#include "isxCore.h"
#include <QFile>
#include <QString>
#include <QXmlStreamReader>
#include <QDateTime>
#include <QLocale>

#include <cmath>
#include <sstream>
#include <string>
#include <vector>


namespace isx
{
    class RecordingXml::Impl
    {
    public:
        /// Constructor for Recording Xml from file
        /// \param inPath to file on disk
        ///
        Impl(const std::string & inPath)
        {

            QFile file(QString::fromStdString(inPath));

            if (!file.exists())
            {
                ISX_THROW(isx::ExceptionFileIO, "The selected file does not exist.");
            }
            
            file.open(QIODevice::ReadOnly | QIODevice::Text);
            QByteArray byteArray = file.readAll();
            file.close();

            QXmlStreamReader reader(byteArray);
            reader.readNext();
            QStringRef name = reader.name();

            // Attributes
            QString start, fps, frames, droppedFrames, width, height, downsample, top, left;
            isize_t totalFrames = 0;

            while (!reader.atEnd())
            {
                reader.readNextStartElement();
                name = reader.name();
                if (name == "recording")
                {
                    while (reader.readNextStartElement())
                    {
                        
                        name = reader.name();
                        if (name == "attrs")
                        {
                            while (reader.readNextStartElement())
                            {
                                QXmlStreamAttributes attrs = reader.attributes();

                                if (attrs.value("name") == "width")
                                {
                                    width = reader.readElementText();
                                }
                                else if (attrs.value("name") == "height")
                                {
                                    height = reader.readElementText();
                                }
                                else if (attrs.value("name") == "left")
                                {
                                    left = reader.readElementText();
                                }
                                else if (attrs.value("name") == "top")
                                {
                                    top = reader.readElementText();
                                }
                                else if (attrs.value("name") == "downsample")
                                {
                                    downsample = reader.readElementText();
                                }
                                else if (attrs.value("name") == "fps")
                                {
                                    fps = reader.readElementText();
                                }
                                else if (attrs.value("name") == "frames")
                                {
                                    frames = reader.readElementText();
                                }
                                else if (attrs.value("name") == "record_start")
                                {
                                    start = reader.readElementText();
                                }                           
                                else if (attrs.value("name") == "dropped")
                                {
                                    droppedFrames = reader.readElementText();
                                }
                                else if (attrs.value("name") == "exposure")
                                {
                                    m_additionalProperties["Exposure (ms)"] = isx::Variant(reader.readElementText().toStdString());
                                } 
                                else if (attrs.value("name") == "led_power")
                                {
                                    m_additionalProperties["LED Power"] = isx::Variant(reader.readElementText().toStdString());
                                }
                                else if (attrs.value("name") == "led_session")
                                {
                                    m_additionalProperties["Total Time LED was ON in Session"] = isx::Variant(reader.readElementText().toStdString());
                                } 
                                else if (attrs.value("name") == "gain")
                                {
                                    m_additionalProperties["Gain"] = isx::Variant(reader.readElementText().toStdString());
                                }
                                else if (attrs.value("name") == "record_sched_name")
                                {
                                    m_additionalProperties["Recording Schedule Name"] = isx::Variant(reader.readElementText().toStdString());
                                } 
                                else if (attrs.value("name") == "version")
                                {
                                    m_additionalProperties["Acquisition SW Version"] = isx::Variant(reader.readElementText().toStdString());
                                }                                
                                else
                                {
                                    reader.skipCurrentElement();
                                }
                            }
                        }
                        else if (name == "decompressed")
                        {
                            m_numFrames.clear();
                            while (reader.readNextStartElement())
                            {
                                QXmlStreamAttributes att = reader.attributes();
                                if (att.hasAttribute("frames"))
                                {
                                    QStringRef sr = att.value("frames");
                                    m_numFrames.push_back(sr.toULong());
                                    totalFrames += m_numFrames.back();
                                }
                                QString filename = reader.readElementText();
                                m_hdf5FileNames.push_back(filename.toStdString());
                            }

                            if (totalFrames != 0)
                            {
                                /// Use totalFrames rather than the number reported under "frames" field, because of nVista bug
                                /// where "frames" reports one extra frame than the actual number
                                frames = QString::number(totalFrames);
                            }
                        }
                        else
                        {
                            reader.skipCurrentElement();
                        }
                    }
                }
            }

            parseDroppedFrames(droppedFrames, m_droppedFrameNums);

            if (!start.isEmpty() &&
                !fps.isEmpty() &&
                !frames.isEmpty())
            {
                initTimingInfo(start, fps, frames, droppedFrames);
            }

            if (!width.isEmpty() &&
                !height.isEmpty() &&
                !downsample.isEmpty() &&
                !top.isEmpty() &&
                !left.isEmpty())
            {
                initSpacingInfo(width, height, downsample, top, left);
            }

        }

        ~Impl() {}

        TimingInfo getTimingInfo()
        {
            return m_timingInfo;
        }

        const std::vector<isize_t> & 
        getDroppedFrames() const
        {
            return m_droppedFrameNums;
        }

        SpacingInfo getSpacingInfo()
        {
            return m_spacingInfo;
        }

        const std::vector<std::string> & getFileNames()
        {
            return m_hdf5FileNames;
        }

        const std::map<std::string, Variant> & getAdditionalProperties() const
        {
            return m_additionalProperties;
        }

        const std::vector<isize_t> & getNumFrames() const
        {
            return m_numFrames;
        }

        void
        serialize(std::ostream& strm) const
        {
            for (isize_t i(0); i < m_hdf5FileNames.size(); ++i)
            {
                strm << m_hdf5FileNames[i] << "\n";
            }
        }

    private: 
        std::vector<std::string> m_hdf5FileNames;
        TimingInfo m_timingInfo;
        SpacingInfo m_spacingInfo;
        std::vector<isize_t> m_droppedFrameNums;    
        std::map<std::string, Variant> m_additionalProperties; 
        /// We store the number of frames in each decompressed file so that
        /// we can directly specify the number of frames
        std::vector<isize_t> m_numFrames;

        void initTimingInfo(const QString & inStart, const QString & inFps, const QString & inNumFrames, const QString & inDroppedFrames)
        {
            Time start;
            DurationInSeconds step;
            isize_t numTimes;

            // Parse starting time
            QString format = "MMM dd, yyyy hh:mm:ss.zzz000 AP";
            QLocale locale(QLocale::English, QLocale::UnitedStates);        // Make sure month names are parsed in English
            QDateTime dt = locale.toDateTime(inStart, format);

            if (!dt.isValid())
            {
                return;
            }

            uint16_t year = (uint16_t) (dt.date().year());
            uint8_t mon = (uint8_t)(dt.date().month());
            uint8_t day = (uint8_t)(dt.date().day());
            uint8_t hour = (uint8_t)(dt.time().hour());
            uint8_t mins = (uint8_t)(dt.time().minute());
            uint8_t secs = (uint8_t)(dt.time().second());
            DurationInSeconds secsOffset = DurationInSeconds((isize_t)(dt.time().msec()), 1000);
            start = Time(year, mon, day, hour, mins, secs, secsOffset);

            // Convert step
            bool ok;
            double fps = inFps.toDouble(&ok);
            if (!ok)
            {
                ISX_THROW(isx::ExceptionDataIO, "Unable to convert frame rate to floating point value.");
            }
            Ratio ratioFps = isx::Ratio::fromDouble(fps);
            step = ratioFps.getInverse();

            // Convert number of frames
            numTimes = inNumFrames.toULongLong();

            // Convert dropped frames 
            std::vector<isize_t> droppedFrameNums;
            parseDroppedFrames(inDroppedFrames, droppedFrameNums);

            numTimes += droppedFrameNums.size();

            m_timingInfo = TimingInfo(start, step, numTimes, droppedFrameNums);

        }

        void initSpacingInfo(const QString & inWidth, const QString & inHeight, const QString & inDownsampleFactor, const QString & inTop, const QString & inLeft)
        {
            // Size in pixels
            isize_t width, height;
            width = inWidth.toULongLong();
            height = inHeight.toULongLong();
            SizeInPixels_t numPixels = SizeInPixels_t(width, height);

            // Pixel size
            int64_t dsFactor = inDownsampleFactor.left(1).toLongLong();
            SizeInMicrons_t pixelSize = SizeInMicrons_t(DEFAULT_PIXEL_SIZE* dsFactor, DEFAULT_PIXEL_SIZE * dsFactor);

            // Top left corner
            PointInMicrons_t topLeft = PointInMicrons_t(inLeft.toLongLong(), inTop.toLongLong());

            m_spacingInfo = SpacingInfo(numPixels, pixelSize, topLeft);

        }

        void parseDroppedFrames(const QString & inDroppedFrames, std::vector<isize_t> & outFrameNums)
        {
            outFrameNums.clear();      

            // Remove the brackets from the ends
            std::string droppedFramesStd = inDroppedFrames.toStdString();
            std::string noBrackets = droppedFramesStd.substr(1, droppedFramesStd.size()-2);

            // Split string (comma-separated values)
            std::stringstream ss(noBrackets);
            std::string s;    
            char delim = ',';
            while (std::getline(ss, s, delim)) 
            {                
                outFrameNums.push_back((isize_t)std::stoull(s));
            }
        }
    };


    RecordingXml::RecordingXml(const std::string & inPath)
    {
        m_pImpl.reset(new Impl(inPath));
    }

    RecordingXml::~RecordingXml()
    {
        m_pImpl.reset();
    }

    const std::vector<std::string> &
    RecordingXml::getFileNames()
    {
        return m_pImpl->getFileNames();
    }
    
    TimingInfo 
    RecordingXml::getTimingInfo()
    {
        return m_pImpl->getTimingInfo();
    }
    const std::vector<isize_t> & 
    RecordingXml::getDroppedFrames() const
    {
        return m_pImpl->getDroppedFrames();
    }

    SpacingInfo 
    RecordingXml::getSpacingInfo()
    {
        return m_pImpl->getSpacingInfo();
    }

    const std::map<std::string, Variant> & 
    RecordingXml::getAdditionalProperties() const
    {
        return m_pImpl->getAdditionalProperties();
    }

    const std::vector<isize_t> &
    RecordingXml::getNumFrames() const
    {
        return m_pImpl->getNumFrames();
    }

    void
    RecordingXml::serialize(std::ostream& strm) const
    {
        m_pImpl->serialize(strm);
    }
}
