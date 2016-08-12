#ifndef ISX_MOVIE_FACTORY_H
#define ISX_MOVIE_FACTORY_H

#include "isxMovie.h"
#include "isxWritableMovie.h"
#include "isxCoreFwd.h"

namespace isx
{

/// Write a new mosaic movie to a file.
///
/// This creates a new movie in a file and initializes the data with
/// zeros. If the file already exists, this will fail.
///
/// \param  inFileName      The name of the movie file.
/// \param  inTimingInfo    The timing information of the movie.
/// \param  inSpacingInfo   The spacing information of the movie.
/// \return                 The mosaic movie created.
///
/// \throw  isx::ExceptionFileIO    If writing the movie file fails.
/// \throw  isx::ExceptionDataIO    If formatting the movie data fails.
SpWritableMovie_t writeMosaicMovie(
        const std::string & inFileName,
        const TimingInfo & inTimingInfo,
        const SpacingInfo & inSpacingInfo);

/// Read an existing movie from a file.
///
/// The actual type of movie to import (e.g. MosaicMovie, NVistaHdf5Movie)
/// is determined by the file extension.
///
/// - isxd: MosaicMovie
/// - hdf5: NVistaHdf5Movie
///
/// If the extension is not recognized, this fails.
///
/// \param  inFileName      The name of the nVista movie file to read.
/// \return                 The imported movie.
///
/// \throw  isx::ExceptionFileIO    If reading the movie file fails.
/// \throw  isx::ExceptionDataIO    If parsing the movie file fails or
///                                 if the extension is not recognized.
SpMovie_t readMovie(const std::string & inFileName);

/// Read an existing mosaic movie from a file.
///
/// \param  inFileName      The name of the mosaic movie file to read.
///
/// \throw  isx::ExceptionFileIO    If reading the movie file fails.
/// \throw  isx::ExceptionDataIO    If parsing the movie file fails.
SpMovie_t readMosaicMovie(const std::string & inFileName);

/// Read an existing nVista HDF5 movie from a file.
///
/// \param  inFileName      The name of the nVista movie file to read.
/// \return                 The imported movie.
///
/// \throw  isx::ExceptionFileIO    If the movie file cannot be read.
/// \throw  isx::ExceptionDataIO    If parsing the movie file failed.
SpMovie_t readNVistaHdf5Movie(const std::string & inFileName);

} // namespace isx

#endif // ifndef ISX_MOVIE_FACTORY_H
