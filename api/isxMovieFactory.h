#ifndef ISX_MOVIE_FACTORY_H
#define ISX_MOVIE_FACTORY_H

#include "isxMovie.h"
#include "isxWritableMovie.h"
#include "isxCoreFwd.h"
#include "isxDataSet.h"
#include "isxVideoFrame.h"

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
/// \param  inHasFrameHeaderFooter  True if written frame data contains fixed size
///                                 header/footer lines. False otherwise.
/// \return                 The mosaic movie created.
///
/// \throw  isx::ExceptionFileIO    If writing the movie file fails.
/// \throw  isx::ExceptionDataIO    If formatting the movie data fails.
SpWritableMovie_t writeMosaicMovie(
        const std::string & inFileName,
        const TimingInfo & inTimingInfo,
        const SpacingInfo & inSpacingInfo,
        DataType inDataType,
        const bool inHasFrameHeaderFooter = false);

/// Read an existing image from a file.
///
/// The actual type of image to import from a movie file (e.g. MosaicMovie, NVistaHdf5Movie)
/// is determined by the file extension.
///
/// - isxd: MosaicMovie
/// - hdf5: NVistaHdf5Movie
///
/// If the extension is not recognized, this fails.
///
/// \param  inFileName      The name of the image file to read.
/// \return                 The imported video frame.
///
/// \throw  isx::ExceptionFileIO    If reading the movie file fails.
/// \throw  isx::ExceptionDataIO    If parsing the movie file fails or
///                                 if the extension is not recognized.
SpVideoFrame_t readImage(const std::string & inFileName);

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
/// \param  inProperties    The properties for start time and frame rate (used only for TIF with no XML)
/// \return                 The imported movie.
///
/// \throw  isx::ExceptionFileIO    If reading the movie file fails.
/// \throw  isx::ExceptionDataIO    If parsing the movie file fails or
///                                 if the extension is not recognized.
SpMovie_t readMovie(const std::string & inFileName, const DataSet::Properties & inProperties = {});

/// Read an existing series of movies from a vector of files.
///
/// The actual type of each movie file to import (e.g. MosaicMovie, NVistaHdf5Movie)
/// is determined by the file extension.
///
/// - isxd: MosaicMovie
/// - hdf5: NVistaHdf5Movie
///
/// If the extension is not recognized for any of the filenames, this fails.
///
/// \param  inFileNames     A vector containing the names of the movie files to read.
/// \param  inProperties    A vector containing the properties of each of the movie files.  Currently only used for behavioral movies and tif with no xml.
/// \return                 The imported movie.
///
/// \throw  isx::ExceptionFileIO    If reading of any of the movie files fails.
/// \throw  isx::ExceptionDataIO    If parsing for any the movie files fails or
///                                 if their extension is not recognized.
SpMovie_t readMovieSeries(const std::vector<std::string> & inFileNames, const std::vector<DataSet::Properties> & inProperties = {});

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
/// \param  inProperties    The properties for start time and frame rate (used only for TIF with no XML)
/// \return                 The imported movie.
///
/// \throw  isx::ExceptionFileIO    If the movie file cannot be read.
/// \throw  isx::ExceptionDataIO    If parsing the movie file failed.
SpMovie_t readInscopixMovie(const std::string & inFileName, const DataSet::Properties & inProperties = {});

/// Read an behavioral movie from a file.
///
/// \param  inFileName      The name of the behavioral movie file to read.
/// \param  inProperties    The properties for this movie, including start time, # frames,
///                         and gopsize (cached so we don't have to scan the file every time
///                         an object is instantiated)
/// \return                 The imported movie.
///
/// \throw  isx::ExceptionFileIO    If the movie file cannot be read.
/// \throw  isx::ExceptionDataIO    If parsing the movie file failed.
SpMovie_t readBehavioralMovie(const std::string & inFileName, const DataSet::Properties & inProperties);

/// Determine if a file is a behavioral movie file by its extension.
///
/// \param  inFileName      The filename to check.
/// \return                 True if filename has an extension for a behavioral movie that is supported.
bool isBehavioralMovieFileExtension(const std::string & inFileName);

/// Determine if a file is an nVista imaging (movie/image) file by its extension.
///
/// \param  inFileName      The filename to check.
/// \return                 True if filename has an extension for nVista imaging data that is supported.
bool isNVistaImagingFileExtension(const std::string & inFileName);

/// Determine if a file is an nVista imaging TIF file by its extension.
///
/// \param  inFileName      The filename to check.
/// \return                 True if filename has a TIF extension.
bool isTiffFileExtension(const std::string & inFileName);

} // namespace isx

#endif // ifndef ISX_MOVIE_FACTORY_H
