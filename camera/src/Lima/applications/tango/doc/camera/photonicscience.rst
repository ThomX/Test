PhotonicScience Tango device
=================

This is the reference documentation of the PhotonicScience Tango device.

you can also find some useful information about the camera models/prerequisite/installation/configuration/compilation in the :ref:`PhotonicScience camera plugin <camera-photonicscience>` section.

Properties
----------

=================== =============== =============== =========================================================================
Property name	    Mandatory	    Default value   Description
=================== =============== =============== =========================================================================
camera_library_path Yes	            N/A             the path to the camera DLL library file
                                                    e.g.: ImageStar4022_v2.5\imagestar4022control.dll
=================== =============== =============== =========================================================================

Attributes
----------
This camera device has no attribute.

Commands
--------

=======================	=============== =======================	===========================================
Command name		Arg. in		Arg. out		Description
=======================	=============== =======================	===========================================
Init			DevVoid 	DevVoid			Do not use
State			DevVoid		DevLong			Return the device state
Status			DevVoid		DevString		Return the device state as a string
getAttrStringValueList	DevString:	DevVarStringArray:	Return the authorized string value list for
			Attribute name	String value list	a given attribute name
=======================	=============== =======================	===========================================
