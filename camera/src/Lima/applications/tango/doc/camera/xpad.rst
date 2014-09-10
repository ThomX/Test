Xpad Tango device
=======

This is the reference documentation of the Xpad Tango device.

you can also find some useful information about the camera models/prerequisite/installation/configuration/compilation in the :ref:`Xpad camera plugin <camera-xpad>` section.

Properties
----------
None.

Attributes
----------
None.

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

