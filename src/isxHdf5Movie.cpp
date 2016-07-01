#include "isxHdf5Movie.h"

namespace isx
{
    Hdf5Movie::Hdf5Movie(const SpH5File_t & inHdf5File, const std::string & inPath)
        : m_H5File(inHdf5File)
        , m_path(inPath)
    {
        try
        {
            // Turn off the auto-printing when failure occurs so that we can
            // handle the errors appropriately
            H5::Exception::dontPrint();
            m_dataSet = m_H5File->openDataSet(m_path);

            m_dataType = m_dataSet.getDataType();
            m_dataSpace = m_dataSet.getSpace();

            isx::internal::getHdf5SpaceDims(m_dataSpace, m_dims, m_maxdims);
            m_ndims = m_dims.size();

            if (!(m_dataType == H5::PredType::STD_U16LE))
            {                
                ISX_THROW(isx::ExceptionDataIO,
                    "Unsupported data type ", m_dataType.getTag());
            }

        }  // end of try block

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

    Hdf5Movie::Hdf5Movie(const SpH5File_t & inHdf5File, const std::string & inPath, isize_t inNumFrames, isize_t inFrameWidth, isize_t inFrameHeight)
        : m_H5File(inHdf5File)
        , m_path(inPath)
    {
        ISX_ASSERT(inNumFrames > 0);
        ISX_ASSERT(inFrameWidth > 0);
        ISX_ASSERT(inFrameHeight > 0);        

        /* Set rank, dimensions and max dimensions */
        m_ndims = 3;
        m_dims.resize(m_ndims);
        m_maxdims.resize(m_ndims);

        m_dims[0] = inNumFrames;
        m_maxdims[0] = inNumFrames;

        m_dims[1] = inFrameHeight;
        m_maxdims[1] = inFrameHeight;

        m_dims[2] = inFrameWidth;
        m_maxdims[2] = inFrameWidth;

        /* Create the dataspace */
        m_dataSpace = H5::DataSpace(static_cast<int>(m_ndims), m_dims.data(), m_maxdims.data());

        /* Create a new dataset within the file */
        m_dataType = H5::PredType::STD_U16LE;
        try
        {
            m_dataSet = isx::internal::createHdf5DataSet(m_H5File, m_path, m_dataType, m_dataSpace);
        }
        catch (const H5::DataSetIException& error)
        {
            ISX_THROW(isx::ExceptionDataIO,
                "Failure caused by H5 DataSet operations.\n", error.getDetailMsg());
        }
        catch (const H5::FileIException& error)
        {
            ISX_THROW(isx::ExceptionFileIO,
                "Failure caused by H5 File operations.\n", error.getDetailMsg());
        }
        catch (const H5::GroupIException& error)
        {
            ISX_THROW(isx::ExceptionDataIO,
                "Failure caused by H5 Group operations.\n", error.getDetailMsg());
        }
    }

    Hdf5Movie::~Hdf5Movie()
    {

    }

    void Hdf5Movie::getFrame(isize_t inFrameNumber, const SpU16VideoFrame_t & vf)
    {
        try 
        {
            isx::internal::HSizeVector_t size = { 1, m_dims[1], m_dims[2] };
            isx::internal::HSizeVector_t offset = { inFrameNumber, 0, 0 };
            H5::DataSpace fileSpace = isx::internal::createHdf5SubSpace(m_dataSpace, offset, size);
            H5::DataSpace bufferSpace = isx::internal::createHdf5BufferSpace(size);

            m_dataSet.read(vf->getPixels(), m_dataType, bufferSpace, fileSpace);
        }
        catch (const H5::DataSetIException& error)
        {
            ISX_LOG_ERROR("Exception in ", error.getFuncName(), ":\n", error.getDetailMsg());
        }
    }

    void Hdf5Movie::writeFrame(isize_t inFrameNumber, void * inBuffer, isize_t inBufferSize)
    {

    }
}