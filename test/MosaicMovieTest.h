#include "isxCore.h"
#include "isxMosaicMovie.h"
#include "catch.hpp"
#include "isxTest.h"

#include <stdio.h>
#include <algorithm>

/// Write out a test movie in U16 format
///
/// \param  inFileName     The name of the file
/// \param  inTimingInfo   Timing info
/// \param  inSpacingInfo  Spacing info
/// \param  inData         Optional parameter pointing to buffer with data
/// \return                Shared pointer to the movie object
isx::SpWritableMovie_t
writeTestU16Movie_dummy(
        const std::string & inFileName,
        const isx::TimingInfo & inTimingInfo,
        const isx::SpacingInfo & inSpacingInfo,
        uint16_t * inData = NULL);

/// Write out a test movie in F32 format
///
/// \param  inFileName     The name of the file
/// \param  inTimingInfo   Timing info
/// \param  inSpacingInfo  Spacing info
/// \param  inData         Optional parameter pointing to buffer with data
/// \return                Shared pointer to the movie object
isx::SpWritableMovie_t
writeTestF32Movie_dummy(
        const std::string & inFileName,
        const isx::TimingInfo & inTimingInfo,
        const isx::SpacingInfo & inSpacingInfo,
        double * inData = NULL);
        
