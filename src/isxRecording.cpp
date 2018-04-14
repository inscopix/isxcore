#include "isxRecording.h"
#include "isxException.h"
#include "isxAssert.h"
#include "isxHdf5FileHandle.h"
#include "isxPathUtils.h"
#include "isxRecordingXml.h"
#include "isxNVistaHdf5Movie.h"
#include "isxNVistaTiffMovie.h"

#include <iostream>
#include <fstream>
#include <QFileInfo>
#include <QDir>

namespace isx {

class Recording::Impl
{
public:
    /// Constructor
    ///
    Impl(){}

    /// Constructor for Recording from file
    /// \param inPath to file on disk
    ///
    Impl(const std::string & inPath, const DataSet::Properties & inProperties)
    : m_path(inPath)
    {
        if (exists())
        {
            std::string extension = inPath.substr(inPath.find_last_of(".") + 1);
            if (extension == "hdf5")
            {
                initializeFromHdf5();
            }
            else if (extension == "xml")
            {
                initializeFromXml();
            }
            else if ((extension == "tif") || (extension == "tiff"))
            {
                DataSet::Properties props(inProperties);
                initializeFromTiff(props);
            }
            else
            {
                ISX_THROW(ExceptionFileIO, "Unsupported file format: ", extension);
            }
        }
        else
        {
            ISX_THROW(isx::ExceptionFileIO,
                "Failed to open unexisting movie file for reading: ", m_path);
        }
    }

    /// Destructor
    ///
    ~Impl(){}

    void initializeFromTiff(DataSet::Properties & inProperties)
    {
        const std::vector<std::string> paths = {m_path};
        isx::Time start;
        if (inProperties.find(DataSet::PROP_MOVIE_START_TIME) != inProperties.end())
        {
            start = inProperties[DataSet::PROP_MOVIE_START_TIME].value<isx::Time>();
        }

        DurationInSeconds step = TimingInfo::s_defaultStep;
        if (inProperties.find(DataSet::PROP_MOVIE_FRAME_RATE) != inProperties.end())
        {
            const double fr = double(inProperties[DataSet::PROP_MOVIE_FRAME_RATE].value<float>());
            step = Ratio::fromDouble(fr).getInverse();
        }

        TimingInfo ti(start, step, 0);
        m_movie = std::make_shared<NVistaTiffMovie>(m_path, paths, ti);

        // no exception until here --> this is a valid file
        m_isValid = true;
    }

    void initializeFromHdf5()
    {
        std::vector<SpHdf5FileHandle_t> fileHandles;
        std::vector<SpH5File_t> files;
        try
        {
            // Turn off the auto-printing when failure occurs so that we can
            // handle the errors appropriately
            H5::Exception::dontPrint();

            files.push_back(std::make_shared<H5::H5File>(m_path.c_str(), H5F_ACC_RDONLY));
            fileHandles.push_back(std::make_shared<Hdf5FileHandle>(files[0], H5F_ACC_RDONLY));
        }  // end of try block

        catch (const H5::FileIException& error)
        {
            ISX_THROW(isx::ExceptionFileIO,
                "Failure caused by H5 File operations.\n", error.getDetailMsg());
        }

        catch (const H5::DataSetIException& error)
        {
            ISX_THROW(isx::ExceptionDataIO,
                "Failure caused by H5 Dataset operations.\n", error.getDetailMsg());
        }

        catch (...)
        {
            ISX_ASSERT(false, "Unhandled exception.");
        }

        m_movie = std::make_shared<NVistaHdf5Movie>(m_path, fileHandles[0]);

        // no exception until here --> this is a valid file
        m_isValid = true;
    }

    void initializeFromXml()
    {
        isx::RecordingXml xml(m_path);

        std::vector<std::string> fileNames = xml.getFileNames();
        const QFileInfo fi(QString::fromStdString(m_path));
        const std::string dir = fi.dir().absolutePath().toStdString();
        for (unsigned int i(0); i < fileNames.size(); ++i)
        {
            fileNames[i] = dir + "/" + fileNames[i];
        }

        TimingInfo ti = xml.getTimingInfo();
        SpacingInfo si = xml.getSpacingInfo();
        std::vector<isize_t> droppedFrames = xml.getDroppedFrames();
        std::map<std::string, Variant> props = xml.getAdditionalProperties();

        if (fileNames.empty())
        {
            ISX_THROW(ExceptionFileIO, "The XML file does not contain any TIFF or HDF5 files to load. ",
                    "You may need to decompress nVista .raw files.");
        }

        std::string extension = isx::getExtension(fileNames.front());

        if(extension == "hdf5")
        {
            std::vector<SpHdf5FileHandle_t> fileHandles;
            std::vector<SpH5File_t> files;
            std::vector<std::string> paths;
            try
            {
                for (unsigned int i(0); i < fileNames.size(); ++i)
                {
                    files.push_back(std::make_shared<H5::H5File>(fileNames[i].c_str(), H5F_ACC_RDONLY));
                    fileHandles.push_back(std::make_shared<Hdf5FileHandle>(files[i], H5F_ACC_RDONLY));
                }
            }
            catch (const H5::FileIException& error)
            {
                ISX_THROW(isx::ExceptionFileIO,
                    "Failure caused by H5 File operations.\n", error.getDetailMsg());
            }

            m_movie = std::make_shared<NVistaHdf5Movie>(m_path, fileHandles, ti, si, droppedFrames, props);
        }
        else if ((extension == "tif") || (extension == "tiff"))
        {
            m_movie = std::make_shared<NVistaTiffMovie>(m_path, fileNames, ti, si, droppedFrames, props, xml.getNumFrames());
        }
        else
        {
            ISX_THROW(ExceptionFileIO, "Unsupported file format: ", extension);
        }

        // no exception until here --> this is a valid file
        m_isValid = true;
    }


    /// \return whether this is a valid internal recording object.
    ///
    bool
    isValid() const
    {
        return m_isValid;
    }

    void
    serialize(std::ostream& strm) const
    {
        strm << m_path;
    }

    SpMovie_t
    getMovie()
    {
        return m_movie;
    }

    std::string 
    getName()
    {
        return m_path;
    }

private:

    bool exists()
    {
        std::ifstream infile(m_path.c_str());
        return infile.good();
    }

    bool m_isValid = false;
    std::string m_path;

    SpMovie_t                       m_movie;
};

Recording::Recording()
{
    m_pImpl.reset(new Impl());
}

Recording::Recording(const std::string & inPath, const DataSet::Properties & inProperties)
{
    m_pImpl.reset(new Impl(inPath, inProperties));
}

Recording::~Recording()
{
}

bool
Recording::isValid() const
{
    return m_pImpl->isValid();
}

void
Recording::serialize(std::ostream& strm) const
{
    m_pImpl->serialize(strm);
}

SpMovie_t
Recording::getMovie()
{
    return m_pImpl->getMovie();
}


std::string 
Recording::getName()
{
    return m_pImpl->getName();
}

} // namespace isx
