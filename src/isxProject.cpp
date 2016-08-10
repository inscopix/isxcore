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
    m_root = std::make_shared<Group>("/");
    SpGroup_t originalGroup = std::make_shared<Group>("Original");
    SpGroup_t outputGroup = std::make_shared<Group>("Output");
    m_root->addGroup(originalGroup);
    m_root->addGroup(outputGroup);
    m_valid = true;
}

Project::~Project()
{
    // TODO sweet : maybe we shouldn't always write when the project is
    // destroyed, but this works for now
    if (m_valid)
    {
        write();
    }
}

SpDataSet_t
Project::createDataSet(
        const std::string & inPath,
        DataSet::Type inType,
        const std::string & inFileName)
{
    std::string name = isx::getFileName(inPath);
    std::string projectDirName = getDirName(m_fileName);
    // TODO sweet : We really want to store a data set's file name as a
    // relative path from the project file's directory
    //std::string relFileName = getRelativePath(projectDirName, inFileName);
    std::string groupPath = getDirName(inPath);
    SpGroup_t group = getGroup(groupPath);
    SpDataSet_t dataSet = group->createDataSet(name, inType, inFileName);
    return dataSet;
}

SpDataSet_t
Project::getDataSet(const std::string & inPath) const
{
    std::string groupPath = getDirName(inPath);
    std::string name = isx::getFileName(inPath);
    SpGroup_t group = getGroup(groupPath);
    SpDataSet_t dataSet = group->getDataSet(name);
    return dataSet;
}

SpGroup_t
Project::getGroup(const std::string & inPath) const
{
    std::vector<std::string> groupNames = getPathTokens(inPath);
    if (groupNames.front() != "/")
    {
        ISX_THROW(isx::ExceptionDataIO,
                  "The project path does not start with the root character (/): ", inPath);
    }
    SpGroup_t currentGroup = m_root;
    std::vector<std::string>::const_iterator it;
    for (it = (groupNames.begin() + 1); it != groupNames.end(); ++it)
    {
        currentGroup = currentGroup->getGroup(*it);
    }
    return currentGroup;
}

SpGroup_t
Project::getRootGroup() const
{
    return m_root;
}

SpGroup_t
Project::getOriginalGroup() const
{
    return m_root->getGroup("Original");
}

SpGroup_t
Project::getOutputGroup() const
{
    return m_root->getGroup("Output");
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
Project::read()
{
    std::ifstream file(m_fileName);
    file.seekg(std::ios_base::beg);
    if (!file.good())
    {
        ISX_THROW(isx::ExceptionFileIO,
                "Error opening project file: ", m_fileName);
    }

    json jsonObject;
    try
    {
        file >> jsonObject;
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

    try
    {
        std::string type = jsonObject["type"];
        if (type.compare("Project") != 0)
        {
            ISX_THROW(isx::ExceptionDataIO,
                    "Expected type to be Project. Instead got ", type);
        }
        m_name = jsonObject["name"];
        m_root = convertJsonToGroup(jsonObject["rootGroup"]);
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
        jsonObject["rootGroup"] = convertGroupToJson(m_root);
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
    if (!file.good())
    {
        ISX_THROW(isx::ExceptionFileIO,
            "Failed to open file when writing project: ", m_fileName);
    }

    file << std::setw(4) << jsonObject;
    if (!file.good())
    {
        ISX_THROW(isx::ExceptionFileIO,
            "Failed to write header in project file: ", m_fileName);
    }
}

} // namespace isx
