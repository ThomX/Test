PCO Tango device
================

This is the reference documentation of the PCO Tango device.

you can also find some useful information about the camera models/prerequisite/installation/configuration/compilation in the :ref:`PCO camera plugin <camera-pco>` section.


Properties
----------

=============== =============== =============== ==============================================================
Property name	Mandatory	Default value	Description
=============== =============== =============== ==============================================================
debug_control   No              0               To set a debug control level (hex value)
debug_module    No              0               To set the debug module list (hex value)
debug_format    No              0               To set the debug format (hex value)
debug_type      No              0               To set the debug type (hex value)	
=============== =============== =============== ==============================================================
For the debug flags you can have a look on the **common/include/Debug.h** file.



Attributes
----------

======================= ======= ======================= ======================================================================
Attribute name          RW      Type                    Description
======================= ======= ======================= ======================================================================
rollingShutter          rw      DevString		Set the rolling shutter mode:
							 - **'only for EDGE'**
							 - **'GLOBAL'**
                                                         - **'ROLLING'**
lastError		ro	DevString		The last PCO error message
camInfo 		ro	DevString		read the *camInfo* takl parameter
camType 		ro	DevString		read the *camType* talk parameter
clXferPar 		ro	DevString		Read the *clTransferParam* talk parameter
cocRunTime 		ro	DevString		Read the *cocRunTime* talk parameter
frameRate 		ro	DevString		Read the *frameRate* talk parameter
pixelRate 		ro	DevString		Read the *pixelRate* talk parameter
maxNbImages             ro	DevString		The maximum number of images which can be acquired by the camera
info     		ro	DevString		Some information (again)
version     		ro	DevString		Read the *timestamp* talk parameter
======================= ======= ======================= ======================================================================

For the above attributes which use "talk" command you can refer to the PCO documentation for more information.

Commands
--------

======================= =============== ======================= ===========================================
Command name            Arg. in         Arg. out                Description
======================= =============== ======================= ===========================================
Init                    DevVoid         DevVoid                 Do not use
State			DevVoid		DevLong			Return the device state
Status			DevVoid		DevString		Return the device state as a string
getAttrStringValueList	DevString:	DevVarStringArray:	Return the authorized string value list for
			Attribute name	String value list	a given attribute name
talk                    DevString       DevString               **WARNING**:use this command for test only, 
                                                                This is a backdoor cmd and it can distrub Lima
=======================	=============== =======================	===========================================
