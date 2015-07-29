.. _Time:

Time
----

A Time object stores a date and time stamp that is used to mark particular
points in time absolute time.

The motivation for this class is that Mosaic 2 is supposed to be based
around can be tricky to take care of, so we better have an object to
handle some of the those difficulties.


Requirements
^^^^^^^^^^^^

- Floating point precision.

  We need this because nVista can recording at floating point frame rates.
  Even if it couldn't, we would still need this because Mosaic should allow
  for temporal downsampling.

- Time definition with respect to a time zone.

  By default this should be UTC, but this should be changed if it's important
  to reflect the local time zone of an experiment.

  This may also be important if a Daylight Savings Time switch occurs during
  an experiment.

- Default construction with some fixed epoch.

  We may wish to define an Inscopix epoch, or just use the Unix epoch of
  midnight on 1970/01/01 UTC.

- Construction using a string representation.

  It should be satisfactory to have one format for construction, which could
  be of the form :code:`YYYYMMDD-hhmmss.z*`, where milliseconds can be given
  up to an arbitrary precision.

  For example::

    Time time("20150512-142947.372")

  would create a :code:`Time` with a date of the 12th of May 2015
  and a time of 14:29:47.372.

  It should also be possible to specify the time zone.

  For example::

    Time time("20150512-142947.372", "PST")

  would create the same time local to Pacific Standard Time (PST).

- Read access to string representations.

  We should supported the same basic format as for construction, but also
  other formats too.

  For example::

    Time time;
    std::cout << time.getString("YYYYMMDD-hhmmss.zzz");

  would print :code:`000000.0000` to standard out and::

    Time time;
    std::cout << time.getString("MMM DD YYYY");

  would print :code:`Jan 01 1970` to standard out.

- Default time zone of UTC/GMT.

  This may be modifiable as a global user preference. Note that nVista does
  not seem to record a time zone, though it probably should.

- Convertible to a chosen time zone.

  For example::

    Time epoch;
    Time epochPst = time.toZone("PST");

  would make :code:`epochPst` have a date of the 31st of December 1969 and
  a time of 16:00:00.000 where :code:`epoch` is the Unix Epoch.

- Add a duration of time in milliseconds to a Time object.

  For example::

    Time time;
    double duration = 2.5;
    Time newTime = time.addMSecs(duration);

- Calculate duration between two Time objects.

  For example::

    Time time1;
    Time time2;
    double duration = time2.mSecsFrom(time1);


Non-Requirements
^^^^^^^^^^^^^^^^

- Need not allow modification of any property.

  Any properties should be set on construction.

