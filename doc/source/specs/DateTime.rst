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

  For example::

    DateTime dateTime;
    std::cout << dateTime.getString("YYYYMMDD-hhmmss-zzz");

  would print :code:`19700101-000000-000` to standard out.

- Creation using string representations.

  For example::

    DateTime dateTime = DateTime::fromString("YYYYMMDD-hhmmss-zzz", "20150512-142947-372")

  would create a :code:`DateTime` with a date of the 12th of May 2015
  and a time of 14:29:47.372.

- Floating point precision.

  We need this because nVista can recording at floating point frame rates.
  Even if it couldn't, we would still need this because Mosaic should allow
  for temporal downsampling.

- Default time zone of UTC/GMT.

  This may be modifiable as a user preference. Note that nVista does not seem
  to record a time zone, though it probably should.

- Convertible to a chosen time zone.

  For example::
    DateTime epoch;
    DateTime epochPst = dateTime.toTimeZone("PST");

  would make :code:`epochPst` have a date of the 31st of December 1969 and
  a time of 16:00:00.000 where :code:`epoch` is the Unix Epoch.

- Add a duration of time in seconds to a DateTime object.

  For example::

    DateTime dateTime;
    double duration = 2.5;
    DateTime newDateTime = dateTime.addSecs(duration);

- Calculate duration between two DateTime objects.

  For example::

    DateTime dateTime1;
    DateTime dateTime2;
    double duration = dateTime2.secsFrom(dateTime1);


Non-Requirements
^^^^^^^^^^^^^^^^

- Need not allow modification of any property.

  Any properties should be set on construction.

