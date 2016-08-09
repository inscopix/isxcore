#include "isxRecordingXml.h"
#include "isxException.h"
#include "isxCore.h"
#include <QFile>
#include <QString>
#include <QXmlStreamReader>
#include <QDateTime>

#include <cmath>

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
            QString start, fps, frames, /*droppedFrames,*/ width, height, downsample, top, left;

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
                                /*else if (attrs.value("name") == "dropped")
                                {
                                    droppedFrames = reader.readElementText();
                                } */                               
                                else
                                {
                                    reader.skipCurrentElement();
                                }
                            }
                        }
                        else if (name == "decompressed")
                        {
                            while (reader.readNextStartElement())
                            {
                                QString filename = reader.readElementText();
                                m_hdf5FileNames.push_back(filename.toStdString());
                            }
                        }
                    }                    
                }
                
            }

            if (!start.isEmpty() &&
                !fps.isEmpty() &&
                !frames.isEmpty() /*&&
                !droppedFrames.isEmpty()*/)
            {
                initTimingInfo(start, fps, frames/*, droppedFrames*/);
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

        SpacingInfo getSpacingInfo()
        {
            return m_spacingInfo;
        }

        const std::vector<std::string> & getFileNames()
        {
            return m_hdf5FileNames;
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

        void initTimingInfo(const QString & inStart, const QString & inFps, const QString & inNumFrames/*, const QString & inDroppedFrames*/)
        {
            Time start;
            DurationInSeconds step;
            isize_t numTimes;

            // Parse starting time
            QString format = "MMM dd, yyyy hh:mm:ss.zzz000 AP";
            QDateTime dt = QDateTime::fromString(inStart, format);
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
            QString integer, fraction;
            integer = inFps.section(".", 0, 0);
            
            isize_t den = 1;
            isize_t fnum = 0;
            if (inFps.contains("."))
            {
                fraction = inFps.section(".", -1, -1);            
                int numDigits = fraction.size();
                den = (isize_t) std::pow(10, numDigits);
                fnum = fraction.toULongLong();
            }

            isize_t inum = integer.toULongLong() * den;            
            isize_t num = inum + fnum;
            step = DurationInSeconds(den, num);     // The den and num correspond to sampling frequency, that's why they are inverted for the step

            // Convert number of frames
            numTimes = inNumFrames.toULongLong();

            m_timingInfo = TimingInfo(start, step, numTimes);

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

    SpacingInfo 
    RecordingXml::getSpacingInfo()
    {
        return m_pImpl->getSpacingInfo();
    }

    void
    RecordingXml::serialize(std::ostream& strm) const
    {
        m_pImpl->serialize(strm);
    }
}