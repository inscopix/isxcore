#include "isxProject.h"
#include "isxException.h"
#include "isxJsonUtils.h"
#include "isxPathUtils.h"

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
    Group * dataSetGroup = parent->createGroup(name, Group::Type::DATASET);
    dataSetGroup->createGroup("derived", Group::Type::DERIVED);
    return dataSetGroup->createDataSet(name, inType, absFileName, inProperties);
}

DataSet *
Project::getDataSet(const std::string & inPath) const
{
    const std::string name = isx::getFileName(inPath);
    const std::string groupPath = getDirName(inPath);
    return getGroup(groupPath)->getDataSet(name);
}

Group *
Project::createGroup(const std::string & inPath)
{
    const std::string name = isx::getFileName(inPath);
    const std::string parentPath = getDirName(inPath);
    Group * parent = getGroup(parentPath);
    return parent->createGroup(name);
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

} // namespace isx
