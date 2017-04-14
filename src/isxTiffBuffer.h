#ifndef ISX_TIFF_BUFFER_H
#define ISX_TIFF_BUFFER_H

#include "isxCore.h"

namespace isx 
{

/// A class to manage a buffer allocated through the LibTiff library
class TIFFBuffer
{
public:
    /// Constructor
    /// \param inBytes size of the buffe in bytes
    TIFFBuffer(isize_t inBytes);
    
    TIFFBuffer(const TIFFBuffer &) = delete;
    
    TIFFBuffer & operator=(const TIFFBuffer &) = delete;

    /// Destructor
    /// 
    ~TIFFBuffer();
    
    /// Get pointer to the buffer memory
    ///
    void * get() const;

private:
    void * m_buf = nullptr;
};

}
#endif // ISX_TIFF_BUFFER_H