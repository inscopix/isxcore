#include "isxExportPNG.h"
#include "isxImage.h"
#include "isxSpacingInfo.h"

#include <qglobal.h>

#include <QImage>
#include <QImageWriter>

namespace isx
{
    void
    toPng(const std::string & inFileName, const SpImage_t & inImage)
    {
        ISX_ASSERT(inImage->getDataType() == DataType::U8);

        QImage outImage = QImage(reinterpret_cast<const uchar*>(inImage->getPixels()), (int)inImage->getWidth(), (int)inImage->getHeight(), (int)inImage->getRowBytes(), QImage::Format_Grayscale8);

        QImageWriter writer(inFileName.c_str());
        writer.setFormat("PNG");

        writer.write(outImage);
    }
} // namespace isx
