#include "isxHdf5Utils.h"
#include "isxException.h"
#include "H5Cpp.h"

namespace isx
{

namespace internal
{

std::vector<std::string>
splitPath(const std::string &s)
{
    using namespace std;
    char delim = '/';
    stringstream ss(s);
    string item;
    vector<string> tokens;
    while (getline(ss, item, delim))
    {
        if(!item.empty())
        {
            tokens.push_back(item);
        }
    }
    return tokens;
}

H5::DataSet
createHdf5DataSet(
    SpH5File_t & file,
    const std::string & name,
    const H5::DataType & dataType,
    const H5::DataSpace & dataSpace)
{
    // Parse name and create the hierarchy tree (if not in the file already).
    // Every level other than the last one is created as a group. The last level is the dataset
    std::vector<std::string> tree = splitPath(name);

    std::string currentObjName("/");
    H5::Group currentGroup = file->openGroup(currentObjName);
    hsize_t nObjInGroup = currentGroup.getNumObjs();

    unsigned int nCreateFromIdx = 0;

    while((nObjInGroup > 0) && (nCreateFromIdx < tree.size()))
    {
        std::string targetObjName = currentObjName + "/" + tree[nCreateFromIdx];
        bool bTargetFound = false;

        for(hsize_t obj(0); obj < nObjInGroup; ++obj)
        {
            std::string objName = currentGroup.getObjnameByIdx(obj);
            if(objName == tree[nCreateFromIdx])
            {
                bTargetFound = true;
                break;
            }
        }

        if(bTargetFound)
        {
            nCreateFromIdx += 1;

            if(nCreateFromIdx < tree.size())
            {
                currentGroup = file->openGroup(targetObjName);
                currentObjName = targetObjName;
                nObjInGroup = currentGroup.getNumObjs();
            }
        }
        else
        {
            break;
        }
    }

    for ( ; nCreateFromIdx < tree.size(); ++nCreateFromIdx)
    {
        if(nCreateFromIdx == (tree.size() - 1))
        {
            return file->createDataSet(name, dataType, dataSpace);
        }

        std::string targetObjName = currentObjName + "/" + tree[nCreateFromIdx];
        file->createGroup(targetObjName);
        currentObjName = targetObjName;
    }

    // If we get here, the dataset exists in the file and we don't need to create it
    H5::Exception::dontPrint();
    H5::DataSet dataSet = file->openDataSet(name);

    H5::DataType readType = dataSet.getDataType();
    H5::DataSpace readSpace = dataSet.getSpace();

    HSizeVector_t dims, maxDims, readDims, readMaxDims;
    getHdf5SpaceDims(dataSpace, dims, maxDims);
    getHdf5SpaceDims(readSpace, readDims, readMaxDims);

    // Check that the size of the file dataset is the same as the one the
    // user is trying to write out
    hsize_t nDims = dims.size();
    hsize_t readNDims = readDims.size();
    if(nDims != readNDims)
    {
        ISX_THROW(isx::ExceptionDataIO,
            "Number of dimensions in requested dataset (", nDims, ") ",
            "does not match that in the read dataset (", readNDims, ").");
    }

    for (hsize_t i(0); i < nDims; i++)
    {
        if(dims[i] != readDims[i])
        {
            ISX_THROW(isx::ExceptionDataIO,
                "Size of dimension ", i, " in requested dataset (", dims[i], ") ",
                "does not that in the read dataset (", readDims[i], ").");
        }

        if(maxDims[i] != readMaxDims[i])
        {
            ISX_THROW(isx::ExceptionDataIO,
                "Max size of dimension ", i, " in requested dataset (", maxDims[i], ") ",
                "does not that in the read dataset (", readMaxDims[i], ").");
        }
    }

    return dataSet;
}

void
getHdf5ObjNames(
    const std::string & inFileName,
    const std::string & inPath,
    std::vector<std::string> & outNames)
{
    H5::Group rootGroup;
    
    try
    {
        H5::H5File file(inFileName.c_str(), H5F_ACC_RDONLY);
        rootGroup = file.openGroup(inPath);
    }
    catch (const H5::FileIException& error)
    {
        ISX_THROW(isx::ExceptionFileIO,
            "Failure caused by H5File operations.\n", error.getDetailMsg());
    }
    
    
    hsize_t nObjInGroup = rootGroup.getNumObjs();

    if (0 == nObjInGroup)
    {
        return;
    }

    outNames.resize(nObjInGroup);
    for (size_t i(0); i < nObjInGroup; ++i)
    {
        outNames[i] = rootGroup.getObjnameByIdx(i);
    }
}

H5::DataSpace
createHdf5SubSpace(
    const H5::DataSpace & space,
    const HSizeVector_t & offset,
    const HSizeVector_t & size)
{
    H5::DataSpace outSpace(space);
    outSpace.selectHyperslab(H5S_SELECT_SET, size.data(), offset.data());
    return outSpace;
}

H5::DataSpace
createHdf5BufferSpace(
    const HSizeVector_t & size)
{
    H5::DataSpace outSpace(3, size.data());
    HSizeVector_t offset = {0, 0, 0};
    outSpace.selectHyperslab(H5S_SELECT_SET, size.data(), offset.data());
    return outSpace;
}

void getHdf5SpaceDims(
    const H5::DataSpace & space,
    HSizeVector_t & dims,
    HSizeVector_t & maxDims)
{
    hsize_t numDims = space.getSimpleExtentNdims();
    dims.resize(numDims);
    maxDims.resize(numDims);
    space.getSimpleExtentDims(dims.data(), maxDims.data());
}

bool hasDatasetAtPath(
    const SpH5File_t & inFile,
    const std::string & inPath,
    const std::string & inDatasetName)
{
    H5::Group rootGroup;
    
    try
    {
        rootGroup = inFile->openGroup(inPath);
    }
    catch (const H5::FileIException& error)
    {
        ISX_THROW(isx::ExceptionFileIO,
            "Failure caused by H5File operation.\n", error.getDetailMsg());
    }

    hsize_t nObjInGroup = rootGroup.getNumObjs();

    if (0 == nObjInGroup)
    {
        return false;
    }

    for (size_t i(0); i < nObjInGroup; ++i)
    {
        if (rootGroup.getObjnameByIdx(i) == inDatasetName)
        {
            return true;
        }
    }

    return false;
}

} // namespace internal

} // namespace isx
