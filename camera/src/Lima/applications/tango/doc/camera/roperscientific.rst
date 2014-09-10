RoperScientific Tango device
===============

This is the reference documentation of the RoperScientific Tango device.

you can also find some useful information about the camera models/prerequisite/installation/configuration/compilation in the :ref:`RoperScientific camera plugin <camera-roperscientific>` section.

Properties
----------

=============== =============== =============== =========================================================================
Property name	Mandatory	Default value	Description
=============== =============== =============== =========================================================================
cam_num		Yes		N/A		The camera number 	
=============== =============== =============== =========================================================================

Attributes
----------
======================= ======= ======================= ======================================================================
Attribute name		RW	Type			Description
======================= ======= ======================= ======================================================================
Temperature		rw	DevFloat	 	The detector read temperature	
TemperatureSetPoint     rw      DevFloat                The detector temperature setpoint
Gain                    rw      DevLong                 The detector gain factor
InternalAcqMode         rw      DevString               The internal acquisition mode, **STANDARD**, **CONTINUOUS** or **FOCUS**
UseFullFrame            rw      DevLong                 ????
======================= ======= ======================= ======================================================================

Please refer to the PVCAM (SDK) user's guide for more information on how to use InternalAcqMode and UseFullFrame attributes.

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
