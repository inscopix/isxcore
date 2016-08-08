#include "isxProject.h"
#include "isxFileUtils.h"
#include "isxException.h"
#include "isxJsonUtils.h"

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
    m_root = std::make_shared<Group>("/");
    SpGroup_t originalGroup = std::make_shared<Group>("Original");
    SpGroup_t outputGroup = std::make_shared<Group>("Output");
    m_root->addGroup(originalGroup);
    m_root->addGroup(outputGroup);
    m_valid = true;
}

Project::~Project()
{
    if (m_valid)
    {
        write();
    }
}

SpGroup_t
Project::getGroup(const std::string & inPath) const
{
    std::vector<std::string> groupNames = getPathTokens(inPath);
    SpGroup_t currentGroup = m_root;
    std::vector<std::string>::const_iterator it;
    for (it = groupNames.begin(); it != groupNames.end(); ++it)
    {
        currentGroup = currentGroup->getGroup(*it);
    }
    return currentGroup;
}

SpDataSet_t
Project::getDataSet(const std::string & inPath) const
{
    std::vector<std::string> pathTokens = getPathTokens(inPath);
    SpGroup_t currentGroup = m_root;
    for (size_t i = 0; i < (pathTokens.size() - 1); ++i)
    {
        currentGroup = currentGroup->getGroup(pathTokens[i]);
    }
    SpDataSet_t dataSet = currentGroup->getDataSet(pathTokens.back());
    return dataSet;
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

} // namespace isx
