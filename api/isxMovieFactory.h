#ifndef ISX_MOVIE_FACTORY_H
#define ISX_MOVIE_FACTORY_H

#include "isxMovie.h"
#include "isxWritableMovie.h"
#include "isxCoreFwd.h"

#include <string>
#include <vector>

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
/// \param  inDataType      The pixel data type of the movie.
/// \return                 The mosaic movie created.
///
/// \throw  isx::ExceptionFileIO    If writing the movie file fails.
/// \throw  isx::ExceptionDataIO    If formatting the movie data fails.
SpWritableMovie_t writeMosaicMovie(
        const std::string & inFileName,
        const TimingInfo & inTimingInfo,
        const SpacingInfo & inSpacingInfo,
        DataType inDataType);

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
/// \param  inFileName      The name of the movie file to read.
/// \return                 The imported movie.
///
/// \throw  isx::ExceptionFileIO    If reading the movie file fails.
/// \throw  isx::ExceptionDataIO    If parsing the movie file fails or
///                                 if the extension is not recognized.
SpMovie_t readMovie(const std::string & inFileName);

/// Read an existing series of movies from a vector of files.
///
/// The actual type of each movie file to import (e.g. MosaicMovie, NVistaHdf5Movie)
/// is determined by the file extension.
///
/// - isxd: MosaicMovie
/// - hdf5: NVistaHdf5Movie
///
/// If the extension is not recognizedfor ane of the filenames, this fails.
///
/// \param  inFileNames     A vector containing the names of the movie files to read.
/// \return                 The imported movie.
///
/// \throw  isx::ExceptionFileIO    If reading of any of the movie files fails.
/// \throw  isx::ExceptionDataIO    If parsing for any the movie files fails or
///                                 if their extension is not recognized.
SpMovie_t readMovieSeries(const std::vector<std::string> & inFileNames);

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

/// Read an behavioral movie from a file.
///
/// \param  inFileName      The name of the behavioral movie file to read.
/// \return                 The imported movie.
///
/// \throw  isx::ExceptionFileIO    If the movie file cannot be read.
/// \throw  isx::ExceptionDataIO    If parsing the movie file failed.
SpMovie_t readBehavioralMovie(const std::string & inFileName);

} // namespace isx

#endif // ifndef ISX_MOVIE_FACTORY_H
