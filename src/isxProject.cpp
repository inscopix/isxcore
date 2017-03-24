#include "isxProject.h"
#include "isxException.h"
#include "isxJsonUtils.h"
#include "isxPathUtils.h"
#include "isxSeriesIdentifier.h"
#include "isxReportUtils.h"

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
        write(m_fileName);
        setUnmodified();
    }
}

void 
Project::saveTmp()
{
    if (m_valid)
    {
        std::string tmp = getTmpFileName();
        write(tmp);
    }
}

SpSeries_t
Project::createDataSetInRoot(
    const std::string & inName,
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
    
    auto s = std::make_shared<Series>(inName, inType, inFileName, inHistory, inProperties);
    DataSet *ds = s->getDataSet(0);
    reportImportData(ds);
    m_root->insertGroupMember(s, m_root->getNumGroupMembers());
    return s;
}

SpSeries_t
Project::createSeriesInRoot(const std::string & inName)
{
    auto s = std::make_shared<Series>(inName);
    m_root->insertGroupMember(s, m_root->getNumGroupMembers());
    return s;
}

bool
Project::canFlattenSeries(
        Series * inSeries,
        std::string & outMessage) const
{
    ISX_ASSERT(inSeries);
    if (inSeries->getNumChildren() > 0)
    {
        outMessage = "Series of processed movies cannot be ungrouped.";
        return false;
    }
    return true;
}

void
Project::flattenSeries(Series * inSeries)
{
    ISX_ASSERT(!inSeries->isUnitary());

    std::string errorMessage;
    if (!canFlattenSeries(inSeries, errorMessage))
    {
        ISX_THROW(ExceptionSeries, errorMessage);
    }

    auto item = inSeries->getContainer();
    ISX_ASSERT(item->getItemType() == ProjectItem::Type::GROUP);
    if (item->getItemType() != ProjectItem::Type::GROUP)
    {
        return;
    }
    auto group = static_cast<Group *>(item);
    // TODO sweet : before moving the items we need to check that the
    // destination doesn't contain any conflicting names.
    auto index = inSeries->getMemberIndex();
    for (auto dataSet : inSeries->getDataSets())
    {
        // create unitary Series and insert into owning group
        SpProjectItem_t newItem = inSeries->removeDataSet(dataSet);
        group->insertGroupMember(newItem, index);
        ++index;
    }
    group->removeGroupMember(inSeries);
}

Series *
Project::findSeriesFromIdentifier(const std::string & inId) const
{
    auto s = SeriesIdentifier::getSeries(inId);
    if (s == nullptr)
    {
        ISX_THROW(ExceptionSeries, "Could not find Series for Id: ", inId);
    }
    return s;
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
Project::setFileName(const std::string & inFileName, bool inFromTemporary)
{   
    if (inFileName == m_fileName)
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

    // Given a ProjectPath and a ProjectName, the directory structure is as follows:
    // ProjectPath/ProjectName              <= projDir
    //     /ProjectName.isxp
    //     /ProjectName_data                <= dataDir
    //         /recording_xyz.isxd
    
    // 1) If coming from temp project, attempt to move old dataDir to new dataDir.
    // 2) If that succeeds -> done (there is no .isxp for temp project).
    // 3) Create new dataDir.
    // 4) Copy data from old dataDir to new project dataDir.
    // 5) If coming from temp project, delete old data files
    // 6) If coming from temp project, delete old dataDir, old temp project file and old projDir.
    // 7) Adjust stored absolute paths in datasets of current project
    
    std::string oldPath = getDataPath();
    std::string prevTmpFileName = getTmpFileName();
    std::string prevName = m_fileName;
    m_fileName = inFileName;
    std::string newPath = getDataPath();

    QDir oldDataDir(QString::fromStdString(oldPath));

    bool wasMoved = false;
    if (inFromTemporary)
    {
        // Step 1
        // Move/rename the data path
        if (oldDataDir.rename(QString::fromStdString(oldPath), QString::fromStdString(newPath)))
        {
            // Remove the old project directory
            std::string oldDirName = getDirName(prevName);
            QDir oldProjDir(QString::fromStdString(oldDirName));
            oldProjDir.rmdir(oldProjDir.absolutePath());
            wasMoved = true;
        }
    }

    if (!wasMoved)
    {
        // Step 3
        initDataDir();
        const QString src = QString::fromStdString(oldPath) + "/";
        const QString dst = QString::fromStdString(newPath) + "/";
        for (QString f : oldDataDir.entryList(QDir::Files))     // maybe only iterate over isxd files that are actually in the project?
        {
            const QString srcFile = src + f;
            const QString dstFile = dst + f;
            // Step 4
            if (!QFile::copy(srcFile, dstFile))
            {
                m_fileName = prevName;
                ISX_THROW(ExceptionFileIO, "Unable to copy data files to: ", newPath);
            }

            // Step 5
            if (inFromTemporary)
            {
                QDir oldProjDir(src);
                bool res = oldProjDir.remove(srcFile);
                if (res == false)
                {
                    ISX_LOG_WARNING("remove ", srcFile.toStdString(), " returned: ", res);
                }
            }
        }

        // Step 6
        if (inFromTemporary)
        {
            std::string oldDirName = getDirName(prevName);
            QDir oldProjDir(QString::fromStdString(oldDirName));
            bool res = oldProjDir.rmdir(oldDataDir.absolutePath());
            if (res == false)
            {
                ISX_LOG_WARNING("rmdir ", oldDataDir.absolutePath().toStdString(), " returned: ", res);
            }
            res = oldProjDir.remove(QString::fromStdString(prevTmpFileName));
            if (res == false)
            {
                ISX_LOG_WARNING("remove ", prevTmpFileName, " returned: ", res);
            }
            res = oldProjDir.rmdir(oldProjDir.absolutePath());
            if (res == false)
            {
                ISX_LOG_WARNING("rmdir ", oldProjDir.absolutePath().toStdString(), " returned: ", res);
            }
        }
    }

    // Update the file paths for data files of project items (Step 7)
    if (oldPath != newPath)
    {
        std::function<void(Series *, const std::string &, const std::string &)> updateSeriesPath =
        [&updateSeriesPath] (
            Series * inSeries,
            const std::string & inOldPath,
            const std::string & inNewPath)
        {
            for (auto & ds : inSeries->getDataSets())
            {
                std::string fn = ds->getFileName();
                if (fn.find(inOldPath) != std::string::npos)
                {
                    fn.replace(0, inOldPath.size(), inNewPath);
                    ds->setFileName(fn);
                }
            }
            for (auto & s : inSeries->getChildren())
            {
                updateSeriesPath(s, inOldPath, inNewPath);
            }
        };

        for (auto & s : getAllSeries())
        {
            updateSeriesPath(s, oldPath, newPath);
        }
    }

    m_name = isx::getBaseName(m_fileName);

}

std::string
Project::createUniquePath(const std::string & inPath) const
{
    return m_root->createUniqueGroupName(inPath);
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
Project::write(const std::string & inFilename) const
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

    std::ofstream file(inFilename, std::ios::trunc);
    writeJson(jsonObject, file);
}

std::string 
Project::getTmpFileName() const
{
    return m_fileName + ".tmp";
}

bool
Project::isFileName(const std::string & inFileName)
{
    for (const auto & s : getAllSeries())
    {
        for (const auto & ds : s->getDataSets())
        {
            if (getAbsolutePath(ds->getFileName()) == inFileName)
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
    
std::vector<Series *>
Project::getAllSeries() const
{
    return getAllSeries(m_root.get());
}

std::vector<Series *>
Project::getAllSeries(const Group * inItem) const
{
    std::vector<Series *> ret;
    for (const auto & m : inItem->getGroupMembers())
    {
        if (m->getItemType() == ProjectItem::Type::GROUP)
        {
            const auto & mm = getAllSeries(static_cast<Group *>(m));
            ret.insert(ret.end(), mm.begin(), mm.end());
        }
        else
        {
            ISX_ASSERT(m->getItemType() == ProjectItem::Type::SERIES);
            ret.push_back(static_cast<Series *>(m));
        }
    }
    return ret;
}

} // namespace isx
