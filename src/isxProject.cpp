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
    Group * origGroup = m_root->createGroup("Original");
    origGroup->createGroup("ImagingData");
    origGroup->createGroup("CellData");
    Group * procGroup = m_root->createGroup("Processed");
    procGroup->createGroup("ImagingData");
    procGroup->createGroup("CellData");
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
Project::createDataSet(
        const std::string & inPath,
        DataSet::Type inType,
        const std::string & inFileName,
        const std::map<std::string, float> & inProperties)
{
    const std::string name = isx::getFileName(inPath);
    // NOTE sweet : when creating a data set through the project, the
    // absolute path gets stored, so that other clients of this data set
    // can use that file name without referring to the path of the project
    // file name.
    std::string absFileName = getAbsolutePath(inFileName);
    const std::string groupPath = getDirName(inPath);
    Group * parent = getGroup(groupPath);
    return parent->createDataSet(name, inType, absFileName, inProperties);
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
    if (groupNames.front() != "/")
    {
        ISX_THROW(isx::ExceptionDataIO,
                  "The project path does not start with the root character (/): ", inPath);
    }
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
Project::getOriginalGroup() const
{
    return m_root->getGroup("Original");
}

Group *
Project::getProcessedGroup() const
{
    return m_root->getGroup("Processed");
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
