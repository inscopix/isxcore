.. _Data:

Data
----

A Data object represents any item in Mosaic that contain data to be processed
by a Mosaic App, such as a movie, an image or a trace.


Requirements
^^^^^^^^^^^^

- A unique name in a Mosaic session.

  This will be used for identifying objects by name. For example, when
  specifying the inputs to an App. Uniqueness should be controlled by the
  :ref:`DataManager`.

  Only read access to the name is required. Changing the name requires
  interaction with the :ref:`DataManager`.

  For example::

    Data* data1 = fromSomewhere();
    String name = data1->getName();

- A unique identifier in a Mosaic project.

  This will be used to reference input and output objects in history.
  This may simply be the name of the object, a unique number within a
  project, or a check sum of the object.

- Store the history of Apps that were run to create it.

  Each app in the history should record all inputs and settings.
  See the :ref:`History` specification for more detail.

  History should not be modifiable. Instead, it should
  be passed as argument during construction of a data object, likely
  during an initialization phase of an App. For example::

    Data* data1 = fromSomewhere();
    App* app = fromSomewhereElse();
    History* history1 = data1->getHistory();
    History* history2 = history1->append(app);
    Data* data2 = dm->createMovie(..., history2);

  If we allow for data objects to come into existence without an
  App (e.g. a manually created trace that annotates the state of an
  animal), then we must allow for a null history.

- Read access to the space required to store the data object.

  This is required so that the user can easily find any large data
  objects that could be removed.

  For example, one could access the size in GB as follows::

    Data* data1 = fromSomewhere();
    double sizeGB = data1.getSize();

- Read access to the location of the data (on a drive or in memory).

  This is required so that the developer can easily see where the data is.
  If the object is on a drive, the file(s) containing it should also be
  accessible.

  For example, one could check if a data object is entirely in memory as
  follows::

    Data* data1 = fromSomewhere();
    String location = data1->getLocation();

- Read access to whether the actual data is somewhere else.

  Data objects can behave a little like a symbolic link, in that the actual
  data may sit on a remote disk that has nothing to do with the local
  filesystem or project directory.


Non-Requirements
^^^^^^^^^^^^^^^^

- Need not know its place in a data object hierarchy.

  This should be controlled by the :ref:`DataManager`. This means that
  the object does not need to know about its parents nor children.

