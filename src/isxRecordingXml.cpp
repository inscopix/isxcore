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

            QXmlStreamReader reader(&file);

            while (!reader.atEnd())
            {
                QStringRef name = reader.name();
                if (name != "decompressed")
                {
                    continue;
                }
                
                while (reader.readNextStartElement())
                {
                    QString filename = reader.readElementText();
                    m_hdf5FileNames.push_back(filename.toStdString());
                }

                break;
            }


        }

        ~Impl() {}


        const std::vector<std::string> & getFileNames()
        {
            return m_hdf5FileNames;
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


}