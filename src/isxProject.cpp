#include "isxProject.h"
#include "isxException.h"
#include "isxJsonUtils.h"
#include "isxPathUtils.h"

#include <fstream>
#include <QDir>
#include <QFile>

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
    
    std::string dataPath = getDataPath();
    QDir dir(QString::fromStdString(dataPath));
    if(!dir.exists())
    {
        ISX_THROW(ExceptionFileIO, "Unable to locate data path: ", dataPath);
    }

    m_name = isx::getBaseName(inFileName);
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
    setUnmodified();
    initDataDir();
    m_valid = true;
}

Project::~Project()
{
    // Remove the project directory if empty
    std::string dataPath = getDataPath();
    QDir dataDir(QString::fromStdString(dataPath));
    
    std::string projPath = isx::getDirName(m_fileName);
    QDir projDir(QString::fromStdString(projPath));
    if(projDir.exists() && (projDir.entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries).count() == 1) && 
        dataDir.exists() && (dataDir.entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries).count() == 0))
    {
        if(!dataDir.rmdir(dataDir.absolutePath()))
        {
            ISX_LOG_ERROR("Unable to delete empty directory: ", dataPath);
        }

        if(!projDir.rmdir(projDir.absolutePath()))
        {
            ISX_LOG_ERROR("Unable to delete empty directory: ", projPath);
        }
        
    }
    else if(projDir.exists() && (projDir.entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries).count() == 0))
    {
        if(!projDir.rmdir(projDir.absolutePath()))
        {
            ISX_LOG_ERROR("Unable to delete empty directory: ", projPath);
        }
    }

}

std::string 
Project::getDataPath() const
{
    return isx::getDirName(m_fileName) + "/" + isx::getBaseName(m_fileName) + "_data";
}

void 
Project::initDataDir()
{
    // Ensure the data path exists
    std::string dataPath = getDataPath();
    QDir dir(QString::fromStdString(dataPath));
    if (!dir.exists())
    {
        if(!dir.mkpath(QString::fromStdString(dataPath)))
        {
            ISX_THROW(ExceptionFileIO, "Unable to create data directory: ", dataPath, ". Verify you have write permissions to this path.");
        }        
    }
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

ProjectItem *
Project::findItem(const std::string & inPath) const
{
    ProjectItem * ret = m_root.get();
    
    ISX_ASSERT(inPath[0] == '/');
    auto path = inPath.substr(1);
    
    while (path != "")
    {
        // extract elements of "path" into variable "name"
        // remove "name" from "path"
        std::string name;
        auto pos = path.find("/");
        if (pos == path.npos)
        {
            name = path;
            path = "";
        }
        else
        {
            name = path.substr(0, pos);
            path = path.substr(pos + 1);
        }

        // search for "name", as historical item first, then children (derived datasets / series members)
        if (ret->getPrevious() && ret->getPrevious()->getName() == name)
        {
            ret = ret->getPrevious();
        }
        else if (ret->isChild(name))
        {
            ret = ret->getChild(name);
        }
        else
        {
            // not found
            ret = nullptr;
            break;
        }
    }

    if (path == "" && ret != nullptr)
    {
        ISX_ASSERT(ret->getPath() == inPath);
    }

    return ret;
}

std::shared_ptr<ProjectItem>
Project::removeItem(const std::string & inPath) const
{
    ProjectItem * item = getItem(inPath);
    ProjectItem * parent = item->getParent();
    ISX_ASSERT(parent != nullptr);
    return parent->removeChild(item->getName());
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
        const HistoricalDetails & inHistory,
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

    auto outDataSet = std::make_shared<DataSet>(name, inType, absFileName, inHistory, inProperties);
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

bool
Project::canFlattenSeries(
        const std::string & inPath,
        std::string & outMessage,
        Series *& outSeries) const
{
    ProjectItem * item = getItem(inPath);
    if (item->getItemType() != ProjectItem::Type::SERIES)
    {
        outMessage = "The requested item is not a series.";
        return false;
    }

    outSeries = static_cast<Series *>(item);
    if (outSeries->hasHistory())
    {
        outMessage = "Series of processed movies cannot be ungrouped.";
        return false;
    }
    return true;
}

void
Project::flattenSeries(const std::string & inPath)
{
    std::string errorMessage;
    Series * series = nullptr;
    if (!canFlattenSeries(inPath, errorMessage, series))
    {
        ISX_THROW(ExceptionSeries, errorMessage);
    }

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

std::string 
Project::getProjectPath() const
{
    return isx::getDirName(m_fileName);
}

void
Project::setFileName(const std::string & inFileName, bool inMoveData)
{   
    if(inFileName == m_fileName)
    {
        return;
    }

    // Make sure the path exists
    std::string dirName = getDirName(inFileName);
    QDir projDir(QString::fromStdString(dirName));
    if (!projDir.exists())
    {
        if(!projDir.mkpath(QString::fromStdString(dirName)))
        {
            ISX_THROW(ExceptionFileIO, "Unable to create project directory: ", dirName, ". Verify you have write permissions to this path.");
        }        
    }

    std::string oldPath = getDataPath();
    std::string prevName = m_fileName;
    m_fileName = inFileName;
    std::string newPath = getDataPath();

    QDir dataDir(QString::fromStdString(oldPath));
    if(inMoveData)
    {
        /// Move/rename the data path        
        if(!dataDir.rename(QString::fromStdString(oldPath), QString::fromStdString(newPath)))
        {
            m_fileName = prevName;
            ISX_THROW(ExceptionFileIO, "Unable to create data directory: ", newPath, ". Verify you have write permissions to this path.");
        }
        // Remove the old project directory
        std::string oldDirName = getDirName(prevName);
        QDir oldProjDir(QString::fromStdString(oldDirName));
        oldProjDir.rmdir(oldProjDir.absolutePath());
    }
    else
    {
        // Copy the data to the new location
        initDataDir();
        QString src = QString::fromStdString(oldPath) + "/";
        QString dst = QString::fromStdString(newPath) + "/";
        for (QString f : dataDir.entryList(QDir::Files)) 
        {         
            if(!QFile::copy(src + f, dst + f))
            {
                m_fileName = prevName;
                ISX_THROW(ExceptionFileIO, "Unable to copy data files to: ", newPath);
            }
        }

    }  

    // Update the file paths for data files of project items 
    if(oldPath != newPath)
    {
        for (auto & item : getAllItems())
        {
            if (item->getItemType() == ProjectItem::Type::DATASET)
            {
                auto dataSet = static_cast<DataSet *>(item);
                std::string fn = dataSet->getFileName();
                if(fn.find(oldPath) != std::string::npos)
                {
                    fn.replace(0, oldPath.size(), newPath);
                    dataSet->setFileName(fn);
                } 

                if(dataSet->hasHistory())
                {
                    // Update historical items 
                    DataSet * prev = (DataSet *) (dataSet->getPrevious());
                    while(prev)
                    {
                        std::string fn = prev->getFileName();
                        if(fn.find(oldPath) != std::string::npos)
                        {
                            fn.replace(0, oldPath.size(), newPath);
                            prev->setFileName(fn);
                        }
                        prev = (DataSet *) (prev->getPrevious());
                    }
                }               
            }
            else if((item->getItemType() == ProjectItem::Type::SERIES) && (item->hasHistory()))
            {
                auto series = static_cast<Series *>(item);

                Series * prev = (Series *) (series->getPrevious());
                while(prev)
                {
                    for(auto & ds : prev->getDataSets())
                    {
                        std::string fn = ds->getFileName();
                        if(fn.find(oldPath) != std::string::npos)
                        {
                            fn.replace(0, oldPath.size(), newPath);
                            ds->setFileName(fn);
                        }
                    }
                    prev = (Series *) (prev->getPrevious());
                }                
            }
        }
    }

    m_name = isx::getBaseName(m_fileName);

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
Project::discard()
{
    std::string projPath = isx::getDirName(m_fileName);
    QDir dir(QString::fromStdString(projPath));
    if(!dir.removeRecursively())
    {
        ISX_LOG_ERROR("Some files could not be removed from: ", projPath);
    }

}

void
Project::read()
{
    std::ifstream file(m_fileName);
    json jsonObject = readJson(file);

    try
    {
        std::string type = jsonObject["type"];
        if (type.compare("Project") != 0)
        {
            ISX_THROW(ExceptionDataIO, "Expected type to be Project. Instead got ", type);
        }
        m_name = jsonObject["name"];
        m_root = Group::fromJsonString(jsonObject["rootGroup"].dump(), getProjectPath());
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
        jsonObject["rootGroup"] = json::parse(m_root->toJsonString(false, getProjectPath()));
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
    writeJson(jsonObject, file);
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
