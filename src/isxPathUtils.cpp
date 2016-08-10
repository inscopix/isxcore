#include "isxPathUtils.h"

#include <QFileInfo>
#include <QDir>
#include <QStringList>
#include <QStandardPaths>

namespace isx
{

std::string
getBaseName(const std::string & inPath)
{
    QFileInfo pathInfo(QString::fromStdString(inPath));
    return pathInfo.baseName().toStdString();
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
    return pathInfo.completeSuffix().toStdString();
}

std::vector<std::string>
getPathTokens(const std::string & inPath)
{
    std::vector<std::string> outPathTokens;
    QString path = QString::fromStdString(inPath);
    //if (path.size() > 0 && path[0] == QChar('/'))
    //{
    //    outPathTokens.push_back("/");
    //}
    QStringList pathTokens = path.split('/', QString::SplitBehavior::SkipEmptyParts);
    for (int i = 0; i < pathTokens.size(); ++i)
    {
        outPathTokens.push_back(pathTokens[i].toStdString());
    }
    return outPathTokens;
}

std::string
getWritableDirName()
{
    QString dirName = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    QDir dir(dirName);
    if (!dir.exists())
    {
        dir.mkpath(dirName);
    }
    return dirName.toStdString();
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

bool
pathExists(const std::string & inPath)
{
    QFileInfo pathInfo(QString::fromStdString(inPath));
    return pathInfo.exists();
}

} // namespace isx
