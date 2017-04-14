#include "isxTiffBuffer.h"
#include <tiffio.h>

namespace isx
{

TIFFBuffer::TIFFBuffer(isize_t inBytes)
{
    m_buf = _TIFFmalloc(tsize_t(inBytes));
}

TIFFBuffer::~TIFFBuffer()
{
    if (m_buf != nullptr)
    {
        _TIFFfree(m_buf);
    }
}

void * TIFFBuffer::get() const
{
    return m_buf;
}

}