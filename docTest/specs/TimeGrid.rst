.. _coreSpecsTimeGrid:

TimeGrid
--------

A TimeGrid stores temporal sampling information that will be used as
the domain for a trace and a movie. Though not really a grid, this does
parallel the SpaceGrid specification and I can't think of a better short
word.


Requirements
^^^^^^^^^^^^

- Read access to start time.

- Read access to end time.

- Read access to temporal spacing in seconds.

  This should correspond to the reciprocal of the frame rate used to
  acquire a recording.

- Read access to number of samples.

- Read access to frame rate in Hz.

  For convenience. Should be derived from spacing.

- Read access to total duration in seconds.

  For convenience. Should be derived from start time, spacing and number of
  samples.

- Read access to relative start, mid, end times of samples (relative or
  absolute).

  For convenience, when a single value needs to represent a time sample, such
  as interpolation or visualization.

- Ability to convert an index to a time (either relative or absolute).

- Ability to convert a time (relative or absolute) to an index.

  This must allow nearest neighbor interpolation. Linear and other
  interpolators should be optional.


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

- :ref:`TraceSpec`: stores a TimeGrid to define its domain as a function of time.

