.. _Image:

Image
-----

An Image represents a scalar valued function of space, for a discrete set of
points.


Requirements
^^^^^^^^^^^^

- Share some basic properties with other data objects.

  For more details, see :ref:`Data`.

- The domain of the function is defined by a top left point and a number of
  evenly spaced points.

  For more details, see :ref:`SpaceGrid`.

- The range of the function is stored in a 2D array, where types may be one of:

    - Boolean (e.g. for the hard definition of an ROI).
    - Integer (e.g. for an image extracted from an integer valued movie).
    - Float (e.g. for a soft definition of a cell).

- Default constructor creates an image with the default :ref:`SpaceGrid`.

- A constructor that allows specification of the :ref:`SpaceGrid`.

  For example::

    SpaceGrid spaceGrid(Point(10, 200), Point(2, 2), Point<int>(720, 540));
    Image image(spaceGrid);

  would create an image with a top left of ``(10, 200)``, a pixel size of
  ``(2, 2)`` microns and ``(720, 540)`` pixels.

- Access to non-const pointer to range array.

  For developer convenience.
  For example::

    Image image;
    float *imageArray = image.getArray();

- Read/write access to the image value array.

  Developers may require direct access to the array for efficiency reasons.
  For example::

    Image image;
    float *imageArray = image.getArray();

- Read/write access to the image values by row/column indices.

  For convenience, this may be achieved by overloading the :code:`[][]`
  operator. For example, the (first, third) value in an image could be
  written and read as follows::

    Image image;
    image[0][3] = 5;
    std::cout << "The value at (0, 3) is " << image[0][3];

- Read access to the image values by a point in space.

  For example, this could be accessed as follows::

    Image float;
    Point point(6.4, 100.7);
    float value = image.getValue(time);
    std::cout << "The value at location " << point << " is " << value;

  This access should at least support nearest neighbor interpolation. We may
  also require bilinear, bicubic or other interpolation.

- Storage and access to multiple channels.

  We need to support dual-color and RGB images.
  While we could define special classes to handle these, I believe it's
  easier to utilize templates and allow the values of images to be fixed
  length arrays (of lengths 2 and 3 in this case) with some suitable
  arithmetic operators defined.

- May need to be memory mapped from disk.

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

- Need not support in-place resampling.

  A separate App should be created to handle resampling of an entire image.


Related Specifications
^^^^^^^^^^^^^^^^^^^^^^

- :ref:`Data` : object will store all basic information about an image.

- :ref:`SpaceGrid` : object will store information about the domain of an
  image.

