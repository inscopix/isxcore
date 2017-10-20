#include "isxPathUtils.h"

#include <iostream>
#include <sstream>
#include <QFileInfo>
#include <QDir>
#include <QStringList>
#include <QStandardPaths>
#include <QStorageInfo>

namespace isx
{

std::string
getBaseName(const std::string & inPath)
{
    QFileInfo pathInfo(QString::fromStdString(inPath));
    return pathInfo.baseName().toStdString();
}

std::string
getFileName(const std::string & inPath)
{
    QFileInfo pathInfo(QString::fromStdString(inPath));
    return pathInfo.fileName().toStdString();
}

std::string
getDirName(const std::string & inPath)
{
    QFileInfo pathInfo(QString::fromStdString(inPath));
    QDir pathDir = pathInfo.dir();
    return pathDir.path().toStdString();
}

std::string
getExtension(const std::string & inPath)
{
    QFileInfo pathInfo(QString::fromStdString(inPath));
    return pathInfo.suffix().toStdString();
}

std::vector<std::string>
getPathTokens(const std::string & inPath)
{
    std::vector<std::string> outPathTokens;
    QString path = QString::fromStdString(inPath);
    if (path.size() > 0 && path[0] == QChar('/'))
    {
        outPathTokens.push_back("/");
    }
    QStringList pathTokens = path.split('/', QString::SplitBehavior::SkipEmptyParts);
    for (int i = 0; i < pathTokens.size(); ++i)
    {
        outPathTokens.push_back(pathTokens[i].toStdString());
    }
    return outPathTokens;
}

std::string
getDefaultProjectPath()
{
    QString dirName = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    dirName += "/Inscopix_Projects";
    QDir dir(dirName);
    if (!dir.exists())
    {
        dir.mkpath(dirName);
    }
    return dirName.toStdString();
}

bool 
isRelative(const std::string &inPath)
{
    std::string dirName = isx::getDirName(inPath);
    QDir dir(QString::fromStdString(dirName));
    return dir.isRelative();
}

std::string
getRelativePath(
        const std::string & inPath,
        const std::string & inDirName)
{
    QDir inDir(QString::fromStdString(inDirName));
    QFileInfo pathInfo(QString::fromStdString(inPath));
    QString relPath = inDir.relativeFilePath(pathInfo.absoluteFilePath());
    return relPath.toStdString();
}

std::string
getAbsolutePath(const std::string & inPath)
{
    QFileInfo pathInfo(QString::fromStdString(inPath));
    return pathInfo.absoluteFilePath().toStdString();
}

bool
pathExists(const std::string & inPath)
{
    QFileInfo pathInfo(QString::fromStdString(inPath));
    return pathInfo.exists();
}

std::string
appendNumberToPath(
        const std::string & inPath,
        const isize_t inNumber,
        const isize_t inWidth)
{
    return inPath + "_" + convertNumberToPaddedString(inNumber, inWidth);
}

bool
makeDirectory(const std::string & inPath)
{
    QDir dir(QString::fromStdString(getDirName(inPath)));
    return dir.mkdir(QString::fromStdString(getBaseName(inPath)));
}

bool
removeDirectory(const std::string & inPath)
{
    QDir dir(QString::fromStdString(inPath));
    return dir.removeRecursively();
}

std::string
makeUniqueFilePath(const std::string & inPath, const isize_t inWidth)
{
    std::string outPath = inPath;
    const std::string base = getDirName(inPath) + "/" + getBaseName(inPath);
    const std::string extension = getExtension(inPath);
    for (isize_t i = 0; pathExists(outPath) && i < 1000; ++i)
    {
        if(extension.empty())
        {
            outPath = appendNumberToPath(base, i, inWidth);
        }
        else
        {
            outPath = appendNumberToPath(base, i, inWidth) + "." + extension;
        }
    }
    return outPath;
}

long long
availableNumberOfBytesOnVolume(const std::string & inDirPath, std::string & outRootDir)
{
    // A path to an as-yet non-existing file may be given.
    // Even the directory potentially could be non-existing before a test runs.
    // Move up the path until an existing subpath is found or until not possible to move up further

    std::string dpStr = inDirPath;
    qint64 numBytes = -1;
    while (true)
    {
        if (numBytes < 0)
        {
            QStorageInfo info = QStorageInfo(QString(dpStr.c_str()));
            numBytes = info.bytesAvailable();
        }
        // Try parent directory in case the sub-directory has not yet been created
        const std::string dpParentStr = getDirName(dpStr);
        if (dpParentStr == dpStr) // getDirName(arg) returns arg if already at top level (cannot go further up)
        {
            outRootDir = dpStr;
            break;
        }
        dpStr = dpParentStr;
    }
    return (long long) numBytes;
}

} // namespace isx
