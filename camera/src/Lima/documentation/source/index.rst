.. Lima documentation master file, created by
   sphinx-quickstart on Fri Feb 18 10:19:02 2011.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

====================================
LIMA : Library for Image Acquisition
====================================
Lima ( **L** ibrary for **Im** age **A** cquisition) is a project for the unified control of 2D detectors. The aim is to clearly separate hardware specific code from common software configuration and features, like setting standard acquisition parameters (exposure time, external trigger), file saving and image processing.

A PDF version of this documentation is available: `pdf documentation`_

Project's Specification is available :ref:`spec`

If you want to be in touch with LIMA community, please send email to lima@esrf.fr. You can also subscribe to the mail-list by sending
a message to sympa@esrf.fr with as subject *subscribe lima*.


.. _compilation:

.. _installation:

Installation
------------

.. toctree::
  :maxdepth: 3


  install_windows
  install_linux
  install_tango_device_server

Howto contribute
----------------

.. toctree::
  :maxdepth: 3

  howto_contribute

Supported Cameras
-----------------

.. toctree::
  :maxdepth: 2

  supported_cameras

Future cameras
--------------

.. toctree::
   :maxdepth: 2

   future_cameras

Applications
------------

.. toctree::
  :maxdepth: 3

  applications/tango/doc/index

Camera plugin developer guide
-----------------------------

.. toctree::
  :maxdepth: 3

  plugin_getting_started/index

`Class documentation`_
----------------------

Indices and tables
------------------

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`

.. _Class documentation: doxygen/html/index.html

.. _pdf documentation: ../latex/Lima.pdf

