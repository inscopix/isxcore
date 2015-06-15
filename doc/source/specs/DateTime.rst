.. _Time:

Time
----

A Time object stores a date/time stamp that is used to mark particular
points in time in a trace or a movie.

The Mosaic 2 GUI is supposed to be based around time. Time can be tricky
to take care of, so we better have an object to handle some of the those
difficulties.


Requirements
^^^^^^^^^^^^

- Read access to string representations of date/time.

  At least a format like "20150512-142947-372". Also things like day of the
  week and month name would be useful.

- Millisecond precision.

  Unlikely to need to be more precise than this. I think it's fast enough
  for electrophysiology input and that's probably the fastest signal we
  have to worry about.

- Accessible w.r.t. a chosen time zone.

  Time stamps should likely be defined w.r.t. UTC, but allowing users to
  choose a working time stamp for visualization would be very useful.

- Add a duration of time in seconds to a Time.

  This would be most convenient as an operator. For example::

    Time time;
    double duration = 2.5;
    Time newTime = time + duration;

- Calculate duration between two Times.

  This would be most convenient as an operator. For example::

    Time time1;
    Time time2;
    double duration = time2 - time1;


Non-Requirements
^^^^^^^^^^^^^^^^

- Need not allow modification of any property.

  Any properties should be set on construction.


