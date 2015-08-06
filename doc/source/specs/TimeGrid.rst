.. _TimeGrid:

TimeGrid
--------

A TimeGrid stores temporal sampling information that will be used as
the domain for a trace and a movie. Though not really a grid, this does
parallel the SpaceGrid specification and I can't think of a better short
word.


Requirements
^^^^^^^^^^^^

- Default constructor creates a time grid starting at the Unix epoch with
  100 samples at 20Hz.

- A constructor that allows specification of the start time, the number of
  times and the step time.

  For example::

    isx::Time start;
    TimeGrid timeGrid(start, 1000, 0.1);

  creates a time grid starting at the default time, with 1000 samples
  separated by 0.1 seconds.

- Read access to start time.

  For example::

    TimeGrid timeGrid;
    Time start = timeGrid.getStart();

- Read access to end time.

  For example::

    TimeGrid timeGrid;
    Time end = timeGrid.getEnd();

- Read access to step time in seconds.

  This should correspond to the reciprocal of the frame rate used to
  acquire a recording.

  For example::

    TimeGrid timeGrid;
    double step = timeGrid.getStep();

- Read access to number of samples.

  For example::

    TimeGrid timeGrid;
    uint32_t numTimes = timeGrid.getNumTimes();

- Read access to sampling or frame rate in Hz.

  Likely for convenience.
  For example::

    TimeGrid timeGrid;
    double rate = timeGrid.getRate();

- Read access to total duration in seconds.

  Likely for convenience.
  For example::

    TimeGrid timeGrid;
    double duration = timeGrid.getDuration();

- Read access to mid times of sample relative to the start.

  For convenience, when a single value needs to represent a time sample, such
  as interpolation or visualization.
  For example::

    TimeGrid timeGrid;
    double* midTimes = timeGrid.getRelMidTimes();

- Ability to convert an index to an absolute mid time.

  This should error if the index is out of bounds.
  For example::

    Time start("20150512-142947");
    double step = 0.05;
    uint32_t numTimes = 100;
    TimeGrid timeGrid(start, step, numTimes);
    uint32_t index = 5;
    Time dateTime = timeGrid.getMidTime(index);

  should make :code:`dateTime` have a time of 14:29:47.275.

- Ability to convert an index to a mid time relative to the start.

  This should error if the index is out of bounds.
  For example::

    Time start("20150512-142947");
    double step = 0.05;
    uint32_t numTimes = 100;
    TimeGrid timeGrid(start, step, numTimes);
    uint32_t index = 5;
    double time = timeGrid.getMidTime(index);

  should make :code:`time` be equal to :code:`0.275`.

- Ability to convert an absolute time to an index.

  This should return the index of the time sample whose center is closest to
  the given time. This should error if the time is not in the domain.
  For example::

    Time start("20150512-142947");
    double step = 0.05;
    uint32_t numTimes = 100;
    TimeGrid timeGrid(start, step, numTimes);
    Time later("20150512-142947.275");
    uint32_t index = timeGrid.getIndex(dateTime);

  should make :code:`index` have a value of :code:`5`.

- Ability to convert a relative time to an index.

  This should return the index of the time sample whose center is closest to
  the given time. This should error if the time is negative or if it is greater than
  the duration of the time grid.
  For example::

    Time start("20150512-142947");
    double step = 0.05;
    uint32_t numTimes = 100;
    TimeGrid timeGrid(start, step, numTimes);
    uint32_t index = timeGrid.getIndex(0.275);

  should make :code:`index` have a value of :code:`5`.


Non-Requirements
^^^^^^^^^^^^^^^^

- Need not allow modification of any property.

  Any properties should be set on construction.

- Need not allow variable spacing.

  Fixed spacing simplifies a lot of operations and visualization.

- Need not store exposure time.

  New hardware may make this into a requirement, but nVista HD doesn't seem
  to need it.

- Need not store spatially varying start time.

  We do have a rolling shutter on nVista HD, but I don't think we store it
  at the moment and it's probably a little difficult to accurately measure.
  It's certainly quite difficult to use.

  If we really cared about this, we should probably just allow these timings
  to be corrected when an nVista recording is loaded.


Related Specifications
^^^^^^^^^^^^^^^^^^^^^^

- :ref:`Time` : represents an absolute point in time.

