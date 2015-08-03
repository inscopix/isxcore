.. _Point:

Point
-----

A Point object simply stores the 2D coordinates of a point in space.

One motivation for this class is to remove the need to always remember to
convert between x/y coordinates and columns/rows of an image matrix.


Requirements
^^^^^^^^^^^^

- Floating point precision.

  I believe the sensor pixel width and height are floating point. Even if
  they're not, we will allow spatial downsampling. Additionally, we want to
  allow continuous coordinates for define regions of interest.

- Default construction gives :code:`(0, 0)`.

- Construction using the x and y coordinates.

  For example::

    Point point(5.2, 108);

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

  would give a new point :code:`(2, 1.5)`.

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

