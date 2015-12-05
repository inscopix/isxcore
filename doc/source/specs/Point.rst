.. _Point:

Point
-----

A Point object simply stores the 2D coordinates of a point in space.

One motivation for this class is to remove the need to always remember to
convert between x/y coordinates and columns/rows of an image matrix.


Requirements
^^^^^^^^^^^^

- Allow both floating point and unsigned integer values.

  We need unsigned integer values for sizes (e.g. number of pixels in each
  dimension) and floating point values for pixel/image widths/heights and
  general continous coordinates (e.g. cell centers or ROI vertices).

  Floating point should be the default.

- Default construction gives ``(0.0, 0.0)``.

- Construction using the x and y coordinates.

  For example::

    Point point(5.2, 108);

- Read access to individual x/y coordinates.

  For example::

    Point point(5.2, 108);
    float x = point.getX();

  makes ``x`` take on the value ``5.2``.

- Read access to array.

  Some developers may prefer direct access to the array.

  For example::

    Point point(5.2, 108);
    float *pointArray = point.getArray();

- Access to array values using the ``[]`` operator.

  This may be convenient for developers.

  For example::

    Point point(5.2, 108);
    float y = point[1];

  would give ``y`` a value of ``108``.

- Access to string representation.

  For example::

    Point point(5.2, 108);
    std::cout << point.getString();

  should print :code:`(5.2, 108)` to standard out.

- Perform point based arithmetic.

  Only needs to handle addition, subtraction, multiplication and division.
  Returns a new point (i.e. not in place arithmetic).

  For example::

    Point point1(5.2, 91);
    Point point2(16, -6.3);
    Point newPoint = point1.plus(point2);

  would give a new point :code:`(21.2, 84.7)`,

  For example::

    Point point1(5.2, 91);
    Point point2(16, -6.3);
    Point newPoint = point1.minus(point2);

  would give a new point :code:`(-10.8, 97.3)`.

  For example::

    Point point1(2, 3);
    Point point2(1, 0.5);
    Point newPoint = point1.times(point2);

  would give a new point :code:`(2, 1.5)`.

  For example::

    Point point1(2, 3);
    Point point2(1, 0.5);
    Point newPoint = point1.divide(point2);

  would give a new point :code:`(2, 6)`.

- Perform scalar based arithmetic.

  Only needs to handle addition, subtraction, multiplication and division.
  Returns a new point (i.e. not in place arithmetic).

  For example::

    Point point1(5.2, 91);
    Point newPoint = point1.plus(4.2);

  would give a new point :code:`(9.4, 95.2)`,

  For example::

    Point point1(5.2, 91);
    Point newPoint = point1.minus(4.2);

  would give a new point :code:`(1, 86.8)`.

  For example::

    Point point1(2, 3);
    Point newPoint = point1.times(0.5);

  would give a new point :code:`(1, 1.5)`.

  For example::

    Point point1(2, 3);
    Point newPoint = point1.divide(0.5);

  would give a new point :code:`(4, 6)`.


Non-Requirements
^^^^^^^^^^^^^^^^

- Need not allow modification of any x/y coordinates.

  These should be set on construction.

