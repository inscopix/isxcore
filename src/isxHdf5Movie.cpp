#include "isxHdf5Movie.h"
#include "isxVideoFrame.h"

namespace isx
{

    const std::string Hdf5Movie::sTimingInfoTimeSecsNum = "TimeSecsNum";
    const std::string Hdf5Movie::sTimingInfoTimeSecsDen = "TimeSecsDen";
    const std::string Hdf5Movie::sTimingInfoTimeOffset = "TimeOffset";
    const std::string Hdf5Movie::sTimingInfoStepNum = "StepNum";
    const std::string Hdf5Movie::sTimingInfoStepDen = "StepDen";
    const std::string Hdf5Movie::sTimingInfoNumTimes = "NumTimes";

    // Only three dimensions are currently supported (frames, rows, columns).
    const hsize_t Hdf5Movie::s_numDims = 3;

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

            hsize_t numDims = m_dims.size();
            if (numDims != s_numDims)
            {
                ISX_THROW(isx::ExceptionDataIO,
                    "Unsupported number of dimensions ", numDims);
            }

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
        m_dims.resize(s_numDims);
        m_maxdims.resize(s_numDims);

        m_dims[0] = inNumFrames;
        m_maxdims[0] = inNumFrames;

        m_dims[1] = inFrameHeight;
        m_maxdims[1] = inFrameHeight;

        m_dims[2] = inFrameWidth;
        m_maxdims[2] = inFrameWidth;

        /* Create the dataspace */
        m_dataSpace = H5::DataSpace(s_numDims, m_dims.data(), m_maxdims.data());

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
        

        // Check that buffer size matches dataspace definition
        if (inBufferSize != getFrameSizeInBytes())
        {
            ISX_THROW(isx::ExceptionUserInput,
                "The buffer size (", inBufferSize, " B) does not match the frame size (",
                getFrameSizeInBytes(), " B).");
        }

        // Check that frame number is within range
        if (inFrameNumber > m_maxdims[0])
        {
            ISX_THROW(isx::ExceptionUserInput,
                "Frame number (", inFrameNumber, ") exceeds the total number of frames (",
                m_maxdims[0], ") in the movie.");
        }

        isx::internal::HSizeVector_t size = { 1, m_dims[1], m_dims[2] };
        isx::internal::HSizeVector_t offset = { inFrameNumber, 0, 0 };
        H5::DataSpace fileSpace = isx::internal::createHdf5SubSpace(
            m_dataSpace, offset, size);
        H5::DataSpace bufferSpace = isx::internal::createHdf5BufferSpace(
            size);

        // Write data to the dataset.
        try
        {
            m_dataSet.write(inBuffer, m_dataType, bufferSpace, fileSpace);
        }

        // Catch failure caused by the DataSet operations
        catch (const H5::DataSetIException &error)
        {
            ISX_THROW(isx::ExceptionDataIO,
                "Failed to write frame to movie.\n", error.getDetailMsg());
        }
    }

    H5::CompType
        Hdf5Movie::getTimingInfoType()
    {
        H5::CompType timingInfoType(sizeof(sTimingInfo_t));
        timingInfoType.insertMember(sTimingInfoTimeSecsNum, HOFFSET(sTimingInfo_t, timeSecsNum), H5::PredType::NATIVE_INT64);
        timingInfoType.insertMember(sTimingInfoTimeSecsDen, HOFFSET(sTimingInfo_t, timeSecsDen), H5::PredType::NATIVE_INT64);
        timingInfoType.insertMember(sTimingInfoTimeOffset, HOFFSET(sTimingInfo_t, timeOffset), H5::PredType::NATIVE_INT32);
        timingInfoType.insertMember(sTimingInfoStepNum, HOFFSET(sTimingInfo_t, stepNum), H5::PredType::NATIVE_INT64);
        timingInfoType.insertMember(sTimingInfoStepDen, HOFFSET(sTimingInfo_t, stepDen), H5::PredType::NATIVE_INT64);
        timingInfoType.insertMember(sTimingInfoNumTimes, HOFFSET(sTimingInfo_t, numTimes), H5::PredType::NATIVE_HSIZE);

        return timingInfoType;
    }

    void Hdf5Movie::readProperties(TimingInfo & timingInfo)
    {
        std::string basePath = m_path.substr(0, m_path.find_last_of("/"));
        std::string propertyPath = basePath + "/Properties";
        H5::DataSet dataset;

        try
        {
            dataset = m_H5File->openDataSet(propertyPath + "/TimingInfo");
        }
        catch (const H5::FileIException& error)
        {
            ISX_THROW(isx::ExceptionFileIO,
                "Failure to read movie properties caused by H5 File operations.\n", error.getDetailMsg());
        }
        catch (const H5::GroupIException& error)
        {
            ISX_THROW(isx::ExceptionDataIO,
                "Failure to read movie properties caused by H5 Group operations.\n", error.getDetailMsg());
        }

        sTimingInfo_t t;
        dataset.read(&t, getTimingInfoType());

        DurationInSeconds secSinceEpoch(t.timeSecsNum, t.timeSecsDen);
        Time start(secSinceEpoch, t.timeOffset);
        DurationInSeconds step(t.stepNum, t.stepDen);
        isize_t numTimes = t.numTimes;
        timingInfo = TimingInfo(start, step, numTimes);
    }

    void Hdf5Movie::writeProperties(TimingInfo & timingInfo)
    {
        /*
        * Initialize the data
        */
        Time time = timingInfo.getStart();
        DurationInSeconds timeSecs = time.getSecsSinceEpoch();
        DurationInSeconds step = timingInfo.getStep();
        sTimingInfo_t t;
        t.timeSecsNum = timeSecs.getNum();
        t.timeSecsDen = timeSecs.getDen();
        t.timeOffset = time.getUtcOffset();
        t.stepNum = step.getNum();
        t.stepDen = step.getDen();
        t.numTimes = timingInfo.getNumTimes();

        /*
        * Create the data space.
        */
        hsize_t dim[] = { 1 };   /* Dataspace dimensions */
        H5::DataSpace space(1, dim);

        try
        {
            /*
            * Create the dataset.
            */
            std::string basePath = m_path.substr(0, m_path.find_last_of("/"));
            std::string grName = basePath + "/Properties";
            H5::Group grProperties = m_H5File->createGroup(grName);
            std::string dataset_name = "TimingInfo";
            H5::DataSet dataset = H5::DataSet(grProperties.createDataSet(dataset_name, getTimingInfoType(), space));
            /*
            * Write data to the dataset;
            */
            dataset.write(&t, getTimingInfoType());
        }

        catch (const H5::DataSetIException &error)
        {
            ISX_THROW(isx::ExceptionDataIO,
                "Failed to write movie properties.\n", error.getDetailMsg());
        }
        catch (const H5::FileIException& error)
        {
            ISX_THROW(isx::ExceptionFileIO,
                "Failure to write movie properties caused by H5 File operations.\n", error.getDetailMsg());
        }
        catch (const H5::GroupIException& error)
        {
            ISX_THROW(isx::ExceptionDataIO,
                "Failure to write movie properties caused by H5 Group operations.\n", error.getDetailMsg());
        }
    }
}
