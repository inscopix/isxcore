#include "isxLog.h"
#include <QFileInfo>

namespace isx
{

namespace internal
{

std::string
baseName(const std::string& fileName)
{
    QFileInfo fileInfo(fileName.c_str());
    return fileInfo.fileName().toStdString();
}

} // namespace internal

} // namespace isx
