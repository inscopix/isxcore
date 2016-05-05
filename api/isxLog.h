#ifndef ISX_LOG_H
#define ISX_LOG_H

#include <iostream>
#include <thread>
#include <QFileInfo>
#include "isxTime.h"

#define ISX_LOG(MSG)\
    QFileInfo fileInfo(__FILE__); \
    std::cout   << fileInfo.fileName().toStdString() << " : " \
                << __LINE__ << " : " \
                << isx::Time::now()->toString() << " : " \
                << std::this_thread::get_id() << " " \
                << MSG << std::endl;
#endif
