.. _DataManager:

Data Manager
------------

The data manager should be a singleton object within one Mosaic session that
manages all data objects, such as movies, images and traces within that
session.

This management should include adding, deleting and moving objects within the
tree, and also allowing some basic queries to select a subset of nodes from
it.

It should also be able to keep reference counts on all the Data objects it
stores.
References may be from the command line interface, from a viewer, or from an
app.
The user should only be able to delete objects when the reference count drops
to zero.


Requirements
^^^^^^^^^^^^

- One data manager per Mosaic session.

  For example this could be accessible using a singleton design pattern::

    DataManager dm = DataManager::getInstance();

- Non-leaf nodes must be some kind of group container.

  This allows for hierarchical structure.

- Leaf nodes must be :ref:`Data` objects.

  I.e. empty groups are not allowed.

- Each node has a unique hierarchical name.

  For example nodes with names ``/a/b/c`` and ``/a/x/c`` would be allowed.

- A node should be accessible by name.

  For example::

    Data data = DataManager::getInstance().get("/a/b/c");

- Add a node with a new name.

  The addition is performed with a requested name, but if that name is
  already taken a different name should be assigned. The actual name
  should be returned.

  For example::

    Data data = fromSomewhere();
    String name = DataManager::getInstance().set("/a/b/c", data);

  Also provide an option to force overwrite. For example::

    DataManager dm = DataManager::getInstance();
    Data data1 = fromSomewhere();
    Data data2 = fromSomewhereElse();
    String name1 = dm.set("/a/b/c", data1);
    String name2 = dm.set("/a/b/c", data2, true);

  then ``name2.equals("/a/b/c")`` is always true.

- Copy one node to another by name.

  This should act like a deep copy of the data.
  This should allow for recursive copy by default. If the destination name
  is already taken, then prevent overwrite and provide the actual name
  assigned.

  For example::

    DataManager dm = DataManager::getInstance();
    Data data = fromSomewhere();
    String name = dm.set("/a/b/c", data);
    String name = dm.copy("/a/b/c", "/a/b/cCopy");

  Also provide an option to force overwrite. For example::

    DataManager dm = DataManager::getInstance();
    Data data1 = fromSomewhere();
    Data data2 = fromSomewhereElse();
    String name1 = dm.set("/a/b/c", data1);
    String name2 = dm.set("/a/b/d", data2);
    String name3 = dm.copy("/a/b/c", "/a/b/d", true);

  then ``name3.equals("/a/b/d")`` is always true.

- Move one node to another by name.

  If the destination name is already taken, then prevent overwrite and
  provide actual name assigned.
  This should act like a rename, and should not make a deep copy of the data.

  For example::

    DataManager dm = DataManager::getInstance();
    Data data = fromSomewhere();
    String name = dm.set("/a/b/c", data);
    String name = dm.move("/a/b/c", "/a/b/d");

  Also provide an option to force overwrite. For example::

    DataManager dm = DataManager::getInstance();
    Data data1 = fromSomewhere();
    Data data2 = fromSomewhereElse();
    String name1 = dm.set("/a/b/c", data1);
    String name2 = dm.set("/a/b/d", data2);
    String name3 = dm.move("/a/b/c", "/a/b/d", true);

  then ``name3.equals("/a/b/d")`` is always true.

- A node should be removable by name.

  This removal should fail if there are other references to this object, such
  as in the command line interface or in a viewer.

  For example::

    DataManager::getInstance().remove("/a/b/c");

- There must be a current working (non-leaf) node.

  This is similar to a current working directory in a file system.
  This would be convenient for access by name within one group.
  Must be able to get and set this, where setting may fail if the
  node does not exist.

  For example::

    DataManager dm = DataManager::getInstance();
    dm.setWorkingNode("/group1/animal1/day1");
    Data data1 = dm.get("recording1");
    Data data2 = dm.get("recording2");

  instead of::

    DataManager dm = DataManager::getInstance();
    Data data1 = dm.get("/group1/animal1/day1/recording1");
    Data data2 = dm.get("/group1/animal1/day1/recording2");


Non-Requirements
^^^^^^^^^^^^^^^^

- No need to have multiple data managers per session.

  This adds complication without any gain.

- No need for the hierarchy in the data manager to be reflected on the
  file system.

  This is not a requirement of the data manager, though we may decide to
  have the data manager be reflected on the file system for reasons of
  usability.

