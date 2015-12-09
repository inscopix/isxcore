.. _Movie:

Movie
-----

A Movie represents a scalar or vector valued function of 2D space, time and
color, for a discrete set of points and times.


Requirements
^^^^^^^^^^^^

- Share some basic properties with other data objects.

  For more details, see :ref:`Data`.

- The spatial domain of the function is defined by a top left point and
  a number of evenly spaced points.

  For more details, see :ref:`SpaceGrid`.
  Only read access is required.

- The temporal domain of the function is defined by an initial time and a
  number of evenly spaced times.

  For more details, see :ref:`TimeGrid`.
  Only read access is required.

- The chromatic domain of the function is defined by a number of channels.

  We need to support scalar, dual-color and RGB movies at least.

- The range of the function can be one of several types.

    - Unsigned 8-bit integer (e.g. RGB movie recorded with a standard camera).
    - Unsigned 16-bit integer (e.g. raw movie recorded with nVista).
    - Single precision floating point (e.g. processed movie).

  The simplest way to achieve this is with templating.
  The default value type should be float with single precision.
  I [Andy] don't think we have a need for double precision floating point.

- Default constructor creates a movie with the default file name,
  :ref:`SpaceGrid`, :ref:`TimeGrid` and a single channel.

- A constructor that allows specification of the file name, :ref:`SpaceGrid`
  and :ref:`TimeGrid` and the number of channels.

  For example::

    std::string fileName = "data/movie1.h5";
    SpaceGrid spaceGrid(Point(10, 200), Point(2, 2), Point<int>(720, 540));
    Time start(2015, 5, 12, 14, 29, 47.372, -8);
    TimeGrid timeGrid(start, 1000, 0.1);
    Movie movie(fileName, spaceGrid, timeGrid);

  would create a movie with a file name of ``data/movie1.h5``, a top left
  corner of ``(10, 200)``, a pixel size of ``(2, 2)`` microns and
  ``(720, 540)`` pixels, and that starts on the 12th of May 2015 at
  14:29:47.372 in time zone UTC-8 with 1000 samples acquired at 10 Hz.

- Allocate a 1D array to hold a given number of frames.

  For example::

    Movie movie;
    float *framesArray = 0;
    int numFrames = 10;
    movie.allocateArray(framesArray, numFrames);

  allocates an array of floats that will be able to store 10 frames worth
  of pixel data associated with the movie.
  Note that the client is responsible for deleting the array.

- Read a sequence of frames by index into a 1D array.

  For example::

    Movie movie;
    float *framesArray;
    int numFrames = 10;
    movie.allocateArray(framesArray, numFrames);
    int startFrame = 0;
    movie.readFrames(framesArray, startFrame, numFrames);

  would read the first 10 frames into the array.
  If the start frame is less than 0, or the number of frames in the movie
  is exceeded, this read should be successful and should fill the array
  with values based on the padding strategy.

- Write a sequence of frames by index.

  For example::

    Movie movie;
    float *framesArray = new float[10];
    int numFrames = 10;
    unsigned int startFrame = 5;
    movie.writeFrames(framesArray, startFrame, numFrames);

  would write 10 frames into the array starting at frame 5.
  The start frame must be at least 0.
  If the number of frames in the movie is exceeded, this should be indicated
  by a return value, but the method should finish.


Non-Requirements
^^^^^^^^^^^^^^^^

- Need not support modification of file name, space grid, time grid or
  number of channels.

  In general, this may require dynamic resizing of the HDF5 file, which
  seems unnecessary.
  I can't think of any case where this is required or would even be
  particularly convenient.


Related Specifications
^^^^^^^^^^^^^^^^^^^^^^

- :ref:`Data` : object will store all basic information about an image.

- :ref:`SpaceGrid` : object will store information about the spatial domain
  of a movie.

- :ref:`TimeGrid` : object will store information about the temporal domain
  of a movie.

