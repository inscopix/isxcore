.. _History:

History
-------

A History object represents the Apps that were run to create a data object.

For example, the following directional graph depicts the history of a trace
that was created by applying an image extracted from NNMF to a movie loaded
from a drive

.. graphviz::

    strict digraph history {
        rankdir = LR;
        loadMovie [ shape = "ellipse" ];
        nnmf [ shape = "ellipse" ];
        applyImage [ shape = "ellipse" ];
        movie [ shape = "box" ];
        image [ shape = "box" ];
        trace [ shape = "box" ];
        loadMovie -> movie -> applyImage -> trace;
        movie -> nnmf -> image -> applyImage -> trace;
    }

where ellipses represent Apps and boxes represent data objects.


Requirements
^^^^^^^^^^^^

- Read access to complete history of Apps.

  This includes all previous Apps until some load or creation App.

- Read access to all settings of Apps.

  These consist of all non-data object inputs to the Apps.

- Read access to input and output data objects of all Apps.

  The inputs and outputs should be referenced by their unique identification
  within a Mosaic project. Ideally, only the relevant inputs and outputs
  would be shown - i.e. those that actually affected how the current data
  object turned out.

