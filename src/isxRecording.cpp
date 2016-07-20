#include "isxRecording.h"
#include "isxException.h"
#include "isxAssert.h"
#include "isxHdf5FileHandle.h"

#include "isxRecordingXml.h"
#include "isxMovie.h"

#include <iostream>
#include <fstream>

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
    Impl(const std::string & inPath)
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
            else
            {
                ISX_THROW(isx::ExceptionFileIO,
                    "Unsupported extension.\n");
            }
        }
    }

    /// Destructor
    ///
    ~Impl(){}

    void initializeFromHdf5()
    {
        try
        {
            // Turn off the auto-printing when failure occurs so that we can
            // handle the errors appropriately
            H5::Exception::dontPrint();

            m_files.push_back(std::make_shared<H5::H5File>(m_path.c_str(), H5F_ACC_RDONLY));
            m_fileHandles.push_back(std::make_shared<Hdf5FileHandle>(m_files[0], H5F_ACC_RDONLY));
            m_movie = std::make_shared<Movie>(m_fileHandles[0], "/images");

            // no exception until here --> this is a valid file
            m_isValid = true;
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
    }

    void initializeFromXml()
    {
        isx::RecordingXml xml(m_path);
        std::vector<std::string> hdf5files = xml.getFileNames();

        if (hdf5files.empty())
        {
            ISX_THROW(isx::ExceptionFileIO, "The file does not contain any HDF5 files to load");
        }
        
        std::vector<std::string> paths;
        try
        {
            std::string dir = m_path.substr(0, m_path.find_last_of("/") + 1);
            for (unsigned int i(0); i < hdf5files.size(); ++i)
            {
                hdf5files[i] = dir + hdf5files[i];
                m_files.push_back(std::make_shared<H5::H5File>(hdf5files[i].c_str(), H5F_ACC_RDONLY));
                m_fileHandles.push_back(std::make_shared<Hdf5FileHandle>(m_files[i], H5F_ACC_RDONLY));
                paths.push_back("/images");
            }
        }
        catch (const H5::FileIException& error)
        {
            ISX_THROW(isx::ExceptionFileIO,
                "Failure caused by H5 File operations.\n", error.getDetailMsg());
        }
        

        m_movie = std::make_shared<Movie>(m_fileHandles, paths);

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
    
    /// \return Hdf5FileHandle
    ///
    SpHdf5FileHandle_t
    getHdf5FileHandle() const
    {
        return m_fileHandles[0];
    }

    /// \return Hdf5FileHandle
    ///
    std::vector<SpHdf5FileHandle_t>
        getHdf5FileHandles() const
    {
        return m_fileHandles;
    }

    void
    serialize(std::ostream& strm) const
    {
        strm << m_path;
    }

    SpMovieInterface_t
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
    
    std::vector<SpH5File_t>         m_files;
    std::vector<SpHdf5FileHandle_t> m_fileHandles;
    SpMovieInterface_t              m_movie;
};

Recording::Recording()
{
    m_pImpl.reset(new Impl());
}

Recording::Recording(const std::string & inPath)
{
    m_pImpl.reset(new Impl(inPath));
}

Recording::~Recording()
{
}

bool
Recording::isValid() const
{
    return m_pImpl->isValid();
}

SpHdf5FileHandle_t
Recording::getHdf5FileHandle()
{
    return m_pImpl->getHdf5FileHandle();
}

std::vector<SpHdf5FileHandle_t>
Recording::getHdf5FileHandles() const
{
    return m_pImpl->getHdf5FileHandles();
}

void
Recording::serialize(std::ostream& strm) const
{
    m_pImpl->serialize(strm);
}

SpMovieInterface_t
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