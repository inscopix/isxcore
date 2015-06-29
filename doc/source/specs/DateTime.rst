.. _DateTime:

DateTime
--------

A DateTime object stores a date/time stamp that is used to mark particular
points in time in a trace or a movie.

The Mosaic 2 GUI is supposed to be based around time. Time can be tricky
to take care of, so we better have an object to handle some of the those
difficulties.


Requirements
^^^^^^^^^^^^

- Read access to string representations.

  At least a format like "20150512-142947-372". Also things like day of the
  week and month name would be useful.

- Creation using string representations.

  At least a format like "20150512-142947-372". Also things like day of the
  week and month name would be useful.

- Millisecond precision.

  Unlikely to need to be more precise than this. I think it's fast enough
  for electrophysiology input and that's probably the fastest signal we
  have to worry about.

- Accessible w.r.t. a chosen time zone.

  Time stamps should likely be defined w.r.t. UTC, but allowing users to
  choose a working time stamp for visualization would be very useful.

- Add a duration of time in seconds to a DateTime object.

  This would be most convenient as an operator. For example::

    DateTime dateTime;
    double duration = 2.5;
    DateTime newDateTime = dateTime + duration;

- Calculate duration between two DateTime objects.

  This would be most convenient as an operator. For example::

    Time dateTime1;
    Time dateTime2;
    double duration = dateTime2 - dateTime1;


Non-Requirements
^^^^^^^^^^^^^^^^

- Need not allow modification of any property.

  Any properties should be set on construction.

