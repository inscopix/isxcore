#include "isxProject.h"
#include "isxException.h"
#include "isxJsonUtils.h"
#include "isxPathUtils.h"
#include "isxMovieFactory.h"
#include "isxMovieSeries.h"

#include <fstream>

namespace isx
{

Project::Project()
    : m_valid(false)    
{
}

Project::Project(const std::string & inFileName)
    : m_valid(false)    
    , m_fileName(inFileName)
{
    read();
    m_valid = true;
    setUnmodified();
}

Project::Project(const std::string & inFileName, const std::string & inName)
    : m_valid(false)    
    , m_name(inName)
    , m_fileName(inFileName)
{
    if (pathExists(inFileName))
    {
        ISX_THROW(isx::ExceptionFileIO,
                "The file name already exists: ", inFileName);
    }
    m_root.reset(new Group("/"));
    m_originalData.reset(new Group("OriginalData"));
    m_valid = true;
    setUnmodified();
}

Project::~Project()
{
}

void
Project::save() 
{
    if (m_valid)
    {
        write();
        setUnmodified();
    }
}

DataSet *
Project::importDataSet(
        const std::string & inPath,
        DataSet::Type inType,
        const std::string & inFileName,
        const DataSet::Properties & inProperties)
{
    const std::string name = isx::getFileName(inPath);
    m_originalData->createDataSet(name, inType, inFileName, inProperties);
    return createDataSet(inPath, inType, inFileName, inProperties);
}

DataSet *
Project::createDataSet(
        const std::string & inPath,
        DataSet::Type inType,
        const std::string & inFileName,
        const DataSet::Properties & inProperties)
{
    const std::string name = isx::getFileName(inPath);
    // NOTE sweet : when creating a data set through the project, the
    // absolute path gets stored, so that other clients of this data set
    // can use that file name without referring to the path of the project
    // file name.
    std::string absFileName = getAbsolutePath(inFileName);
    const std::string groupPath = getDirName(inPath);
    Group * parent = getGroup(groupPath);
    ISX_ASSERT(parent != nullptr);

    // TODO sweet : if the parent is a series, then check that the new data
    // set can be added to it and find the index at which it should be
    // inserted
    int index = -1;
    if (parent->getType() == Group::Type::SERIES)
    {
        checkDataSetForSeries(parent, inType, absFileName);
        index = findSeriesIndex(parent, inType, absFileName);
    }

    Group * dataSetGroup = parent->createGroup(name, Group::Type::DATASET, index);
    dataSetGroup->createGroup("derived", Group::Type::DERIVED);
    try
    {
        return dataSetGroup->createDataSet(name, inType, absFileName, inProperties);
    }
    catch (const Exception & error)
    {
        (void)error;
        parent->removeGroup(name);
        throw;
    }
}

DataSet *
Project::getDataSet(const std::string & inPath) const
{
    const std::string name = isx::getFileName(inPath);
    return getGroup(inPath)->getDataSet(name);
}

Group *
Project::createGroup(
        const std::string & inPath,
        const Group::Type inType,
        const int inIndex)
{
    const std::string name = isx::getFileName(inPath);
    const std::string parentPath = getDirName(inPath);
    Group * parent = getGroup(parentPath);
    if (parent->getType() == Group::Type::SERIES)
    {
        if (inType != Group::Type::DATASET)
        {
            ISX_THROW(ExceptionSeries, "A series group can only contain data set groups.");
        }
    }
    return parent->createGroup(name, inType, inIndex);
}

void
Project::moveGroups(
        const std::vector<std::string> & inSrcs,
        const std::string & inDest,
        const int inIndex)
{
    Group * dest = getGroup(inDest);

    if (dest->getType() == Group::Type::SERIES)
    {
        // Must check all data sets can be moved into group before moving them
        std::vector<const DataSet *> srcDataSets;
        for (const auto & srcPath : inSrcs)
        {
            const Group * src = getGroup(srcPath);
            if (src->getType() != Group::Type::DATASET)
            {
                ISX_THROW(ExceptionSeries, "Only data set groups can be added to a series.");
            }

            // NOTE sweet : if the data set is already in the series then ignore the move
            if ((src->getParent() == dest) && (dest->isGroup(src->getName())))
            {
                continue;
            }
            const DataSet * srcDs = src->getDataSetFromGroup();
            ISX_ASSERT(srcDs != nullptr);
            checkDataSetForSeries(dest, srcDs->getType(), srcDs->getFileName());
            srcDataSets.push_back(srcDs);
        }
        checkDataSetsForSeries(srcDataSets);

        // Move the data sets group one by one and insert them in order.
        // NOTE sweet : it would probably be more efficient to construct the group in
        // any order and then sort, but as I already have findSeriesIndex, I decided
        // to do it this way.
        for (auto ds : srcDataSets)
        {
            Group * parent = ds->getParent();
            ISX_ASSERT(parent != nullptr);
            Group * grandParent = parent->getParent();
            ISX_ASSERT(grandParent != nullptr);
            const int index = findSeriesIndex(dest, ds->getType(), ds->getFileName());
            grandParent->moveGroup(ds->getName(), dest, index);
        }
        return;
    }

    for (const auto & srcPath : inSrcs)
    {
        Group * src = getGroup(srcPath);
        Group * srcParent = src->getParent();
        if (srcParent != nullptr)
        {
            srcParent->moveGroup(src->getName(), dest, inIndex);
        }
    }
}

void
Project::flattenGroup(const std::string & inPath)
{
    Group * group = getGroup(inPath);
    if (group == m_root.get())
    {
        ISX_THROW(ExceptionDataIO, "The root group cannot be flattened.");
    }
    Group * parent = group->getParent();
    ISX_ASSERT(parent != nullptr);
    int index = group->getIndex();
    for (auto subGroup : group->getGroups())
    {
        group->moveGroup(subGroup->getName(), parent, index);
        if (index != -1)
        {
            index++;
        }
    }
    // NOTE sweet : technically the group can immediately contain
    // data sets so I need to move those too
    for (auto dataSet : group->getDataSets())
    {
        group->moveDataSet(dataSet->getName(), parent);
        if (index != -1)
        {
            index++;
        }
    }
    parent->removeGroup(group->getName());
}

Group *
Project::getGroup(const std::string & inPath) const
{
    const std::vector<std::string> groupNames = getPathTokens(inPath);
    Group * currentGroup = m_root.get();
    std::vector<std::string>::const_iterator it;
    for (it = (groupNames.begin() + 1); it != groupNames.end(); ++it)
    {
        currentGroup = currentGroup->getGroup(*it);
    }
    return currentGroup;
}

Group *
Project::getRootGroup() const
{
    return m_root.get();
}

Group *
Project::getOriginalDataGroup() const
{
    return m_originalData.get();
}

bool
Project::isValid() const
{
    return m_valid;
}

std::string
Project::getName() const
{
    return m_name;
}

std::string
Project::getFileName() const
{
    return m_fileName;
}

void 
Project::setFileName(const std::string & inFileName)
{
    m_fileName = inFileName;
}

bool
Project::isPath(const std::string & inPath) const
{
    const std::vector<DataSet *> dataSets = m_root->getDataSets(true);
    for (auto it = dataSets.begin(); it != dataSets.end(); ++it)
    {
        if ((*it)->getPath() == inPath)
        {
            return true;
        }
    }
    const std::vector<Group *> groups = m_root->getGroups(true);
    for (auto it = groups.begin(); it != groups.end(); ++it)
    {
        if ((*it)->getPath() == inPath)
        {
            return true;
        }
    }
    return false;
}

std::string
Project::createUniquePath(const std::string & inPath) const
{
    std::string outPath = inPath;
    for (isize_t i = 0; isPath(outPath) && i < 1000; ++i)
    {
        outPath = appendNumberToPath(inPath, i, 3);
    }
    return outPath;
}

bool 
Project::isModified() const
{
    return m_root->isModified();
}

bool
Project::isGroup(const std::string & inPath) const
{
    const std::vector<std::string> pathTokens = getPathTokens(inPath);
    const size_t numTokens = pathTokens.size();
    if (numTokens == 0)
    {
        return false;
    }
    if (numTokens == 1)
    {
        return pathTokens[0] == "/";
    }
    bool exists = true;
    Group * group = getGroup(pathTokens[0]);
    for (size_t i = 0; exists && i < (numTokens - 1); ++i)
    {
        if (group->isGroup(pathTokens[i + 1]))
        {
            group = group->getGroup(pathTokens[i + 1]);
        }
        else
        {
            exists = false;
        }
    }
    return exists;
}

void
Project::read()
{
    std::ifstream file(m_fileName);
    json jsonObject = readJsonHeader(file, false);

    try
    {
        std::string type = jsonObject["type"];
        if (type.compare("Project") != 0)
        {
            ISX_THROW(isx::ExceptionDataIO,
                    "Expected type to be Project. Instead got ", type);
        }
        m_name = jsonObject["name"];
        m_root = createProjectTreeFromJson(jsonObject["rootGroup"]);
        m_originalData = createProjectTreeFromJson(jsonObject["originalDataGroup"]);
    }
    catch (const std::exception & error)
    {
        ISX_THROW(isx::ExceptionDataIO,
                "Error while parsing project header: ", error.what());
    }
    catch (...)
    {
        ISX_THROW(isx::ExceptionDataIO,
                "Unknown error while parsing project header.");
    }
}

void
Project::write() const
{
    json jsonObject;
    try
    {
        jsonObject["type"] = "Project";
        jsonObject["name"] = m_name;
        jsonObject["mosaicVersion"] = CoreVersionVector();
        jsonObject["rootGroup"] = convertGroupToJson(m_root.get());
        jsonObject["originalDataGroup"] = convertGroupToJson(m_originalData.get());
    }
    catch (const std::exception & error)
    {
        ISX_THROW(isx::ExceptionDataIO,
                "Error generating project header: ", error.what());
    }
    catch (...)
    {
        ISX_THROW(isx::ExceptionDataIO,
            "Unknown error while generating project header.");
    }

    std::ofstream file(m_fileName, std::ios::trunc);
    writeJsonHeader(jsonObject, file, false);    
}

void 
Project::setUnmodified()
{
    m_root->setUnmodified();
}

void
Project::checkDataSetForSeries(const DataSet::Type inType)
{
    if (inType != DataSet::Type::MOVIE)
    {
        ISX_THROW(ExceptionSeries, "A series can only contain nVista movies.");
    }
}

void
Project::checkDataSetForSeries(
        const DataSet * inRef,
        const DataSet::Type inType,
        const std::string & inFileName)
{
    checkDataSetForSeries(inType);

    const DataSet::Type refType = inRef->getType();
    if (inType != refType)
    {
        ISX_THROW(ExceptionSeries,
                "The data set has a different type than the reference data set.");
    }

    const SpMovie_t refMovie = readMovie(inRef->getFileName());
    const SpMovie_t movie = readMovie(inFileName);

    MovieSeries::checkDataType(refMovie->getDataType(), movie->getDataType());
    MovieSeries::checkSpacingInfo(refMovie->getSpacingInfo(), movie->getSpacingInfo());
    MovieSeries::checkTimingInfo(refMovie->getTimingInfo(), movie->getTimingInfo());
}

void
Project::checkDataSetForSeries(
           const Group * inSeries,
           const DataSet::Type inType,
           const std::string & inFileName)
{
    ISX_ASSERT(inSeries->getType() == Group::Type::SERIES);
    std::vector<Group *> dsGroups = inSeries->getGroups();

    if (dsGroups.size() == 0)
    {
        checkDataSetForSeries(inType);
        return;
    }

    for (const auto dsGroup : dsGroups)
    {
        const DataSet * ds = dsGroup->getDataSetFromGroup();
        checkDataSetForSeries(ds, inType, inFileName);
    }
}

void
Project::checkDataSetsForSeries(const std::vector<const DataSet *> & inDataSets)
{
    if (inDataSets.size() == 0)
    {
        return;
    }

    // Use the first data set as a reference
    const DataSet * refDs = *inDataSets.begin();
    checkDataSetForSeries(refDs->getType());
    const Group * refParent = refDs->getParent();
    ISX_ASSERT(refParent != nullptr);
    const Group * refGrandParent = refParent->getParent();
    ISX_ASSERT(refGrandParent != nullptr);

    // Check that the others are consistent with the reference
    for (auto it = inDataSets.begin() + 1; it != inDataSets.end(); ++it)
    {
        const isx::DataSet * ds = *it;
        checkDataSetForSeries(refDs, ds->getType(), ds->getFileName());
        const Group * parent = ds->getParent();
        ISX_ASSERT(parent != nullptr);
        const Group * grandParent = parent->getParent();
        ISX_ASSERT(grandParent != nullptr);
        if (grandParent != refGrandParent)
        {
            ISX_THROW(ExceptionSeries,
                    "The data set has a different parent than the reference data set.");
        }
    }
}

int
Project::findSeriesIndex(
        const Group * inSeries,
        const DataSet::Type inType,
        const std::string & inFileName)
{
    ISX_ASSERT(inSeries->getType() == Group::Type::SERIES);
    checkDataSetForSeries(inType);

    const SpMovie_t newMovie = readMovie(inFileName);
    const Time newStart = newMovie->getTimingInfo().getStart();
    int outIndex = 0;
    for (auto dsGroup : inSeries->getGroups())
    {
        const DataSet * ds = dsGroup->getDataSetFromGroup();
        ISX_ASSERT(ds != nullptr);
        const SpMovie_t movie = readMovie(ds->getFileName());
        if (newStart < movie->getTimingInfo().getStart())
        {
            break;
        }
        ++outIndex;
    }
    return outIndex;
}

} // namespace isx
