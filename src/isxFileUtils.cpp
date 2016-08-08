#include "isxFileUtils.h"

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

bool
doesPathExist(const std::string & inPath)
{
    QFileInfo pathInfo(QString::fromStdString(inPath));
    return pathInfo.exists();
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

} // namespace isx
