.. _Data:

Data
----

A Data object represents any item in Mosaic that contain data to be processed
by a Mosaic App, such as a movie, an image or a trace.


Requirements
^^^^^^^^^^^^

- A unique name in a Mosaic session.

  This will be used for identifying objects by name. For example, when
  specifying the inputs to an App.

- A unique identifier in a Mosaic project.

  This will be used to reference input and output objects in history.
  This may simply be the name of the object, a unique number within a
  project, or a check sum of the object.

- Store the history of Apps run to create it.

  See the :ref:`Data` specification for more detail.

- Read access to the space required to store the data object.

  This is required so that the user can easily find any large data
  objects that could be removed.

- Read access to the location of the data (on a drive or in memory).

  This is required so that the user can easily see where the data
  is. If the object is on a drive, the file(s) containing it should
  also be accessible.


Non-Requirements
^^^^^^^^^^^^^^^^

- Need not know its place in a data object hierarchy.

  There will be a separate data object manager that is responsible for this
  hierarchy. This means that the object does not need to know about its
  parents or children.

