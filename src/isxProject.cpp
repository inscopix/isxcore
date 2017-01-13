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
        ISX_THROW(ExceptionFileIO, "The file name already exists: ", inFileName);
    }
    m_root = std::make_shared<Group>("/");
    m_valid = true;
    setUnmodified();
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

ProjectItem *
Project::getItem(const std::string & inPath) const
{
    const std::vector<std::string> itemNames = getPathTokens(inPath);
    if (itemNames.front() != "/")
    {
        ISX_THROW(ExceptionDataIO, "All items paths must be absolute.");
    }
    ProjectItem * outItem = m_root.get();
    for (auto it = (itemNames.begin() + 1); it != itemNames.end(); ++it)
    {
        outItem = outItem->getChild(*it);
    }
    return outItem;
}

void
Project::removeItem(const std::string & inPath) const
{
    ProjectItem * item = getItem(inPath);
    ProjectItem * parent = item->getParent();
    ISX_ASSERT(parent != nullptr);
    parent->removeChild(item->getName());
}

void
Project::moveItem(
        const std::string & inSrc,
        const std::string & inDest,
        const int inIndex)
{
    const std::string parentPath = getDirName(inSrc);
    const std::string itemName = getBaseName(inSrc);
    ProjectItem * parent = getItem(parentPath);
    ISX_ASSERT(parent != nullptr);

    const int origIndex = parent->getChild(itemName)->getIndex();
    std::shared_ptr<ProjectItem> item = parent->removeChild(itemName);

    ProjectItem * dest = getItem(inDest);

    // NOTE sweet : the insertion might fail depending on the destination
    // so we must put it back if that's the case.
    try
    {
        dest->insertChild(item, inIndex);
    }
    catch (...)
    {
        parent->insertChild(item, origIndex);
        throw;
    }
}

DataSet *
Project::createDataSet(
        const std::string & inPath,
        const DataSet::Type inType,
        const std::string & inFileName,
        const DataSet::Properties & inProperties)
{
    // NOTE sweet : when creating a data set through the project, the
    // absolute path gets stored, so that other clients of this data set
    // can use that file name without referring to the path of the project
    // file name.
    std::string absFileName = getAbsolutePath(inFileName);
    if (isFileName(absFileName))
    {
        ISX_THROW(ExceptionFileIO, "There is already a data set with the file name: ", absFileName);
    }

    const std::string groupPath = getDirName(inPath);
    const std::string name = getBaseName(inPath);
    ProjectItem * parent = getItem(groupPath);
    ISX_ASSERT(parent != nullptr);

    auto outDataSet = std::make_shared<DataSet>(name, inType, absFileName, inProperties);
    parent->insertChild(outDataSet);
    return outDataSet.get();
}

Series *
Project::createSeries(const std::string & inPath, const int inIndex)
{
    const std::string groupPath = getDirName(inPath);
    const std::string name = getBaseName(inPath);
    ProjectItem * parent = getItem(groupPath);
    ISX_ASSERT(parent != nullptr);

    auto series = std::make_shared<Series>(name);
    parent->insertChild(series, inIndex);
    return series.get();
}

void
Project::flattenSeries(const std::string & inPath)
{
    ProjectItem * item = getItem(inPath);
    if (item->getItemType() != ProjectItem::Type::SERIES)
    {
        ISX_THROW(ExceptionDataIO, "The requested item is not a series.");
    }

    Series * series = static_cast<Series *>(item);
    ProjectItem * parent = series->getParent();
    // TODO sweet : before moving the items we need to check that the
    // destination doesn't contain any conflicting names.
    int index = series->getIndex();
    for (auto dataSet : series->getDataSets())
    {
        std::shared_ptr<DataSet> ds = series->removeDataSet(dataSet->getName());
        parent->insertChild(ds, index);
        ++index;
    }
    parent->removeChild(series->getName());
}

Group *
Project::getRootGroup() const
{
    return m_root.get();
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
    for (const auto & item : getAllItems())
    {
        if (item->getPath() == inPath)
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
            ISX_THROW(ExceptionDataIO, "Expected type to be Project. Instead got ", type);
        }
        m_name = jsonObject["name"];
        m_root = Group::fromJsonString(jsonObject["rootGroup"].dump());
    }
    catch (const std::exception & error)
    {
        ISX_THROW(ExceptionDataIO, "Error while parsing project header: ", error.what());
    }
    catch (...)
    {
        ISX_THROW(isx::ExceptionDataIO, "Unknown error while parsing project header.");
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
        jsonObject["rootGroup"] = json::parse(m_root->toJsonString());
    }
    catch (const std::exception & error)
    {
        ISX_THROW(ExceptionDataIO, "Error generating project header: ", error.what());
    }
    catch (...)
    {
        ISX_THROW(ExceptionDataIO, "Unknown error while generating project header.");
    }

    std::ofstream file(m_fileName, std::ios::trunc);
    writeJsonHeader(jsonObject, file, false);
}

bool
Project::isFileName(const std::string & inFileName)
{
    for (const auto & item : getAllItems())
    {
        if (item->getItemType() == ProjectItem::Type::DATASET)
        {
            auto dataSet = static_cast<const DataSet *>(item);
            if (dataSet->getFileName() == inFileName)
            {
                return true;
            }
        }
    }
    return false;
}

void
Project::setUnmodified()
{
    m_root->setUnmodified();
}

std::vector<ProjectItem *>
Project::getAllItems() const
{
    return getAllItems(m_root.get());
}

std::vector<ProjectItem *>
Project::getAllItems(const ProjectItem * inItem) const
{
    std::vector<ProjectItem *> outItems;
    for (const auto & child : inItem->getChildren())
    {
        outItems.push_back(child);
        std::vector<ProjectItem *> grandChildren = getAllItems(child);
        outItems.insert(outItems.end(), grandChildren.begin(), grandChildren.end());
    }
    return outItems;
}

} // namespace isx
