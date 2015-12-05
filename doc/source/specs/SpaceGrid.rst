.. _SpaceGrid:

SpaceGrid
---------

A SpaceGrid stores spatial sampling information that will be used as
the domain for an image and the partial domain of a movie.

As we do not, and perhaps cannot, spatially calibrate the imaging data,
the spatial domain is assumed to be restricted to the physical space taken
up by the sensor. In that sense, the pixel width and height correspond to
the pixel width and height of the sensor pixels.


Requirements
^^^^^^^^^^^^

- Default constructor creates nVista sensor space grid.

  I.e.::

    SpaceGrid spaceGrid;

  creates a space grid with a top left of (0, 0), a pixel size of (1, 1)
  and (1440, 1080) pixels.

- Fully specified constructor takes top left, pixel size and number of
  pixels.

  For example::

    SpaceGrid spaceGrid(Point(-5, 200), Point(2, 2), Point<int>(720, 540));

  creates a space grid with a top left of ``(100, 200)``, a pixel size of
  ``(2, 2)`` microns and ``(720, 540)`` pixels. This represents a space grid
  that might be a result of downsampling and cropping an nVista movie.

- Read access to top left corner.

  For example::

    SpaceGrid spaceGrid;
    Point topLeft = spaceGrid.getTopLeft();

- Read access to the bottom right corner.

  For example::

    SpaceGrid spaceGrid;
    Point end = spaceGrid.getBottomRight();

- Read access to the pixel size in microns.

  This is returned as a 2D point, where the coordinates represent
  the width and height of the pixel in microns.

  For example::

    SpaceGrid spaceGrid;
    Point pixelSize = spaceGrid.getPixelSize();

- Read access to the number of pixels in each dimension.

  This is returned as a 2D point, where the coordinates should be
  integral and represents the number of pixels in the x-dimension
  (number of columns) and the number of pixels in the y-dimension
  (number of rows).

  For example::

    SpaceGrid spaceGrid;
    Point<uint32_t> numPixels = spaceGrid.getNumPixels();

- Read access to centers of pixels in a 1D array.

  Pixel centers are typically used as the representative points associated
  with pixels. This will be useful for resampling and visualization.

  The indices in the array correspond to the row-wise vectorized indices
  of the corresponding matrix of indices. E.g. :math:`(0, 0) \mapsto 0`,
  :math:`(0, 2) \mapsto 2X + 1`, where :math:`X` is the number of pixels
  in the x-dimension, and :math:`(2, 0) \mapsto 2` .

  For example::

    SpaceGrid spaceGrid;
    std::vector<Point> pixelCenters = spaceGrid.getPixelCenters();

  should make :code:`pixelCenters[2]` be :code:`(2.5, 0.5)`.

- Ability to convert an index to a pixel center.

  The 1D index corresponds to the row-wise vectorized index of the
  corresponding 2D matrix of indices. E.g. :math:`0 \mapsto (0, 0)`,
  :math:`(0, 2) \mapsto 2X + 1`, where :math:`X` is the number of pixels in
  the x-dimension and :math:`2 \mapsto (2, 0)`.
  This should error if the index is out of bounds.

  For example::

    SpaceGrid spaceGrid;
    Point point = spaceGrid.getPixelCenter(2);

  should make :code:`point` be :code:`(2.5, 0.5)`.

- Ability to convert a 2D point to an index.

  This should return the index of the pixel whose center is closest to
  the given point, but should error if the time is not in the domain.
  For example::

    SpaceGrid spaceGrid;
    uint32_t index = spaceGrid.getPointIndex(Point(2.5, 0.5));

  should make :code:`index` have a value of :code:`2`.


Non-Requirements
^^^^^^^^^^^^^^^^

- Need not allow modification of any property.

  Any properties should be set on construction.


Related Specifications
^^^^^^^^^^^^^^^^^^^^^^

- :ref:`Point` : represents a 2D point in space.

