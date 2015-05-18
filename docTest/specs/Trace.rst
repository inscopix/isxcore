.. _coreSpecsTrace:

Trace
-----

A Trace represents a scalar valued function of time, for a discrete set
of times.

Requirements
^^^^^^^^^^^^

- Share some basic properties with other data objects.

  For more details, see :ref:`coreSpecsData`.

- The domain of the function is defined by an initial time and a number of
  evenly spaced times.

  For more details, see :ref:`coreSpecsTimeGrid`.

- The range of the function is stored in a 1D array, where types may be one of:

    - Boolean.
    - Enumeration (e.g. state of animal).
    - Integer (e.g. for traces extracted from a raw movie).
    - Float (e.g. for traces extracted from a processed movie).

- Read/write access to the range array by index.

  For convenience, this may be achieved by overloading the :code:`[]`
  operator. For example, the third value in a trace could be written and
  read as follows::

    Trace<int> trace;
    trace[2] = 5;
    std::cout << "The third value is " << trace[2];

  In the case that this operator turns out to be a bottleneck, we may require
  pointer access to the array.

- Read access to the range array by time.

  For example, this could be accessed as follows::

    Trace<int> trace;
    Time time;
    int value = trace.getValue(time);
    std::cout << "The value at time " << time << " is " << value;

  This access should at least support nearest neighbor interpolation. We may
  also require linear or other interpolation.


Non-Requirements
^^^^^^^^^^^^^^^^

- Need not stored multiple channels.

  This may need to be relaxed, but I can't think of many traces that would
  be created from multi-channel movies. Even in cases where it's plausible,
  such as for a dual color microscope, we could simply create two traces.

- Need not be memory mapped to disk.

  Even very long traces will be relatively small in size. For example, a
  trace containing 1 hour of data acquired at 20 Hz will contain

  .. math::
    3.6 \times 10^3 \mathrm{s} \cdot 2 \times 10 \mathrm{Hz} = 7.2 \times 10^4

  values. Assuming each value is a floating point single precision number,
  this would require

  .. math::
    4 \cdot 7.2 \times 10^4 \mathrm{B} &= 3 \times 10^5 \mathrm{B}
    &\approx 300 \mathrm{KB}

  of storage space. At worst, we should only need to store 1000 neurons
  simultaneously, which would require

  .. math::
    3 \times 10^5 \mathrm{KB} \cdot 10^3 &= 3 \times 10^8 \mathrm{KB}
    &\approx 300 \mathrm{MB}

  of storage space. Even if the 1 hour length or 20 Hz frame rate of this
  example is off by an order of magnitude, we can still expect to only
  consume around 3 GB of space, which is well contained by our recommended
  machine specifications.

- Need not support dynamical resizing.

  This includes decreases and increases in size anywhere in the trace.
  I can't think of any case where this is required or would even be
  particularly convenient.

- Need not support in-place resampling.

  A separate App should be created to handle resampling of an entire trace.


Related Specifications
^^^^^^^^^^^^^^^^^^^^^^

- :ref:`coreSpecsData`: object will store all basic information about a trace.

- :ref:`coreSpecsTimeGrid`: object will store information about the domain of a trace.

