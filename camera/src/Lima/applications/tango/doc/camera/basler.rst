Basler Tango device
======

This is the reference documentation of the Basler Tango device.

you can also find some useful information about the camera models/prerequisite/installation/configuration/compilation in the :ref:`Basler camera plugin <camera-basler>` section.

Properties
----------

======================== =============== =============== ==============================================================
Property name	         Mandatory	 Default value	 Description
======================== =============== =============== ==============================================================
cam_ip_address	         Yes		 N/A		 The camera's ip or hostname 
inter_packet_delay       No              0               The inter packet delay
frame_transmission_delay No              0               The frame transmission delay
======================== =============== =============== ==============================================================

Both inter_packet_delay and frame_tranmission_delay properties can be used to tune the GiGE performance, for
more information on how to configure a GiGE Basler camera please refer to the Basler documentation.


Attributes
----------

This camera device has not attribute.


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


