.. _Image:

Image
-----

An Image represents a scalar or vector valued function of space, for a
discrete set of points.


Requirements
^^^^^^^^^^^^

- Share some basic properties with other data objects.

  For more details, see :ref:`Data`.

- The domain of the function is defined by a top left point and a number of
  evenly spaced points.

  For more details, see :ref:`SpaceGrid`.

- The chromatic domain of the function is defined by a number of channels.

  We need to support scalar, dual-color and RGB movies at least.

- The range of the function can be one of several types.

    - Boolean (e.g. for the hard definition of an ROI).
    - Integer (e.g. for an image extracted from an integer valued movie).
    - Single precision floating point (e.g. for a soft definition of a cell).

  The simplest way to achieve this is with templating.
  The default value type should be float with single precision.
  I [Andy] don't think we have a need for double precision floating point.

- Default constructor creates an image with the default :ref:`SpaceGrid`.

- A constructor that allows specification of the :ref:`SpaceGrid`.

  For example::

    SpaceGrid spaceGrid(Point(10, 200), Point(2, 2), Point<int>(720, 540));
    Image image(spaceGrid);

  would create an image with a top left of ``(10, 200)``, a pixel size of
  ``(2, 2)`` microns and ``(720, 540)`` pixels.

- Access to non-const pointer to 3D array of image data.

  Clients may require direct access to the array for efficiency reasons.
  For example::

    Image image;
    float *imageArray = 0;
    image.getArray(imageArray);

  If the image is actually stored on disk, the client will be responsible
  for deleting the resulting array.
  

- May need to be stored on disk.

  The sensor size of nVista is ``(1440, 1080)``.
  I don't believe sensor sizes will get larger in the products we will be
  offering in the next few years, so serves as a good upper bound.

  That means a full resolution image will contain

  .. math::
    1.44 \times 10^3 \mathrm{s} \cdot 1.08 \times 3 \approx 1.5 \times 10^6

  values.
  Assuming each value is a floating point single precision number, this would
  require

  .. math::
    4 \cdot 1.5 \times 10^6 \mathrm{B} = 6 \times 10^6 \mathrm{B} \approx 6 \mathrm{MB}

  of storage space.
  For one animal, we may need to store 1000 images (representing 1000
  neurons) simultaneously, which would require about 6 GB.

  At most, we should expect to store 10 animals simultaneously, so in the
  worst case we will require about 60 GB.

  Note that in more standard cases, the images have been cropped and
  downsampled from their original extent and resolution.
  This means that we might expect a reduction by a factor of 10 in a more
  usual case to give a memory requirement of about 6 GB, which would be
  more acceptable.


Non-Requirements
^^^^^^^^^^^^^^^^

- Need not support modification of space grid.

  In general, this may require dynamic resizing of the data array, which
  seems unnecessary.
  I can't think of any case where this is required or would even be
  particularly convenient.
  Similarly changing the top left corner or the pixel sizes does not seem to
  be required.


Related Specifications
^^^^^^^^^^^^^^^^^^^^^^

- :ref:`Data` : object will store all basic information about an image.

- :ref:`SpaceGrid` : object will store information about the domain of an
  image.

