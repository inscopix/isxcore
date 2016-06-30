#include "isxRecordingXml.h"
#include "isxException.h"
#include <QFile>
#include <QString>
#include <QXmlStreamReader>

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

            while (!reader.atEnd())
            {
                reader.readNextStartElement();
                name = reader.name();
                if (name != "recording")
                {
                    continue;
                }
                else
                {
                    while (reader.readNextStartElement())
                    {
                        name = reader.name();
                        if (name != "decompressed")
                        {
                            reader.skipCurrentElement();
                            continue;
                        }
                        else
                        {
                            while (reader.readNextStartElement())
                            {
                                QString filename = reader.readElementText();
                                m_hdf5FileNames.push_back(filename.toStdString());
                            }
                        }
                    }
                    break;
                }
                
            }


        }

        ~Impl() {}


        const std::vector<std::string> & getFileNames()
        {
            return m_hdf5FileNames;
        }

        void
        serialize(std::ostream& strm) const
        {
            for (int i(0); i < m_hdf5FileNames.size(); ++i)
            {
                strm << m_hdf5FileNames[i] << "\n";
            }
        }

    private: 
        std::vector<std::string> m_hdf5FileNames;
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

    void
    RecordingXml::serialize(std::ostream& strm) const
    {
        m_pImpl->serialize(strm);
    }
}