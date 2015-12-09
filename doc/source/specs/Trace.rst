.. _Trace:

Trace
-----

A Trace represents a scalar valued function of time, for a discrete set
of times.

Requirements
^^^^^^^^^^^^

- Share some basic properties with other data objects.

  For more details, see :ref:`Data`.

- The domain of the function is defined by an initial time and a number of
  evenly spaced times.

  For more details, see :ref:`TimeGrid`.

- The range of the function is stored in a 1D array, where types may be one of:

    - Boolean.
    - Enumeration (e.g. state of animal).
    - Integer (e.g. for traces extracted from a raw movie).
    - Float (e.g. for traces extracted from a processed movie).

  Support for these types is likely best achieved using templating.
  In this case the default type should be floating point.

- Default constructor creates an image with the default :ref:`TimeGrid`.

- A constructor that allows specification of the :ref:`TimeGrid` domain.

  For example::

    Time start(2015, 5, 12, 14, 29, 47.372, -8);
    TimeGrid timeGrid(start, 1000, 0.1);
    Trace trace(timeGrid);

  would create a ``Trace`` that starts on the 12th of May 2015
  at 14:29:47.372 in time zone UTC-8 with 1000 samples acquired in 10 Hz.

- Access to non-const pointer to range array.

  For developer convenience.
  For example::

    Trace trace;
    float *traceArray = trace.getArray();

- Read/write access to the range array by index.

  For convenience, this may be achieved by overloading the ``[]``
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

- Need not be stored on disk.

  Even very long traces will be relatively small in size. For example, a
  trace containing 1 hour of data acquired at 20 Hz will contain

  .. math::
    3.6 \times 10^3 \mathrm{s} \cdot 2 \times 10 \mathrm{Hz} = 7.2 \times 10^4

  values. Assuming each value is a floating point single precision number,
  this would require

  .. math::
    4 \cdot 7.2 \times 10^4 \mathrm{B} = 3 \times 10^5 \mathrm{B} \approx 300 \mathrm{KB}

  of storage space.
  For one animal, we may need to store 1000 traces (corresponding to 1000 traces)
  simultaneously, which would require

  .. math::
    3 \times 10^2 \mathrm{KB} \cdot 10^3 = 3 \times 10^8 \mathrm{KB} \approx 300 \mathrm{MB}

  of storage space.

  At most, we should expect to store the neuron traces of 10 animals
  simultaneously, so we can expect to use about 3 GB of memory, which is well
  contained by our machine specification.

  Also note that is likely convenient to store traces in physical memory
  because large portions may be simultaneously visualized in the GUI, which
  then may be analyzed in an App.

- Need not support modification of time grid.

  In general, this may require dynamic resizing of the data array, which
  seems unnecessary.
  I can't think of any case where this is required or would even be
  particularly convenient.
  Similarly changing the start time or sampling rate does not seem to be
  required.

- Need not support in-place resampling.

  A separate App should be created to handle resampling of an entire trace.


Related Specifications
^^^^^^^^^^^^^^^^^^^^^^

- :ref:`Data` : object will store all basic information about a trace.

- :ref:`TimeGrid` : object will store information about the domain of a trace.

