Andor Tango device
=============

This is the reference documentation of the Andor Tango device.

you can also find some useful information about prerequisite/installation/configuration/compilation in the :ref:`Andor camera plugin <camera-andor>` section.

Properties
----------

=============== =============== =============== =========================================================================
Property name	Mandatory	Default value	Description
=============== =============== =============== =========================================================================
camera_number	No		N/A		The camera number,  default is  0	
config_path	No		N/A		The configuration path, for linux default is /usr/local/etc/andor	
cooler		No		Off		Start/stop the cooling system of the camera mode	
fast_trigger	No		Off		Fast external trigger mode, see Andor documentation for usage	
shutter_level	No		High		The shutter output level mode
temperature_sp	No		N/A		The temperature setpoint in Celsius
p_gain  	No		-1	 	The preamplifier gain [0-N] (see detector spec.), -1 to set to maxgain	
vs_speed  	No		-1		The vertical shift speed, index of speed [0-N], -1 to set fasten	
adc_speed  	No		-1		The adc/Horiz. speed pair index [0-N], -1 to set max speed	
=============== =============== =============== =========================================================================



Attributes
----------
======================= ======= ======================= ======================================================================
Attribute name		RW	Type			Description
======================= ======= ======================= ======================================================================
cooler			rw	DevString		Start/stop the cooling system of the camera mode:
							 - **On**, the cooler is started
							 - **Off**, the cooler is stopped 	
cooling_status		ro	DevString		The status of the cooling system, tell if the setpoint 
							temperature is reached
fast_trigger		rw	DevString		Fast external trigger mode, see Andor documentation for usage Mode are:
							 - **On**, fast mode, the camera will not wait until the a 
							   keep clean cycle has been completed before accepting the next 
							   trigger
							 - **Off**, slow mode	
shutter_level		rw	DevString		The shutter output level mode:
							 - **Low"**, output TTL low signal to open shutter
							 - **High**, output TTL high signal to open shutter
temperature		ro	DevShort	 	The current sensor temperature in Celsius	
temperature_sp		rw	DevShort		The temperature setpoint in Celsius
timing			ro	Spectrum		The exposure and latency times	
p_gain  		rw	DevShort	 	The preamplifier gain [0-N] (see detector spec.), -1 to set to maxgain	
vs_speed  		rw	DevShort	 	The vertical shift speed, index of speed [0-N], -1 to set fasten 
							recommended speed
adc_speed  		rw	DevShort		The adc/Horiz. speed pair index [0-N], -1 to set max speed 
======================= ======= ======================= ======================================================================

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

