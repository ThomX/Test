RayonixHs Tango device
===========

This is the reference documentation of the RayonixHs Tango device.

you can also find some useful information about the camera models/prerequisite/installation/configuration/compilation in the :ref:`RayonixHs camera plugin <camera-rayonixhs>` section.


Properties
----------

=========================== =============== =============== =========================================================================
Property name               Mandatory       Default value   Description
=========================== =============== =============== =========================================================================
frame_mode                  No              single          The frame mode, **single** or **fast_transfer**
frame_trigger_signal_type   No              opto            The frame trigger signal type (input #1)
sequence_gate_signal_type   No              opto            The gate signal type (input #2)
electronic_shutter_enabled  No              false           The electronic shutter **true** or **false** to activate or not
cooler_temperature_setpoint No              -120            The cooling system temperature setpoint in Celsuis
sensor_temperature_setpoint No              -80             The detector (sensor) temperature setpoint in Celsuis
output1_signal_type         No              cmos            The output #1 signal type
output2_signal_type         No              cmos            The output #2 signal type
output1_id                  No              shutter         The output #1 signal source
output2_id                  No              frame           the output #2 signal source
=========================== =============== =============== =========================================================================

The Rayonix HS input/output system supports different type of signals:
 - OPTO/OPTO_INVERTED/CMOS/CMOS_PULLDOWN/CMOS_PULLUP/CMOS_PULLDOWN_INVERTED/CMOS_PULLUP_INVERTED

And it provides a output multiplexer for both outputs within the following list of sources:
 - SHUTTER/INTEGRATE/FRAME/LINE/SHUTTER_OPENING/SHUTTER_CLOSING/SHUTTER_ACTIVE/TRIGGER_RISE_WAIT/TRIGGER_RISE_ACK/TRIGGER_FALL_WAIT/TRIGGER_FALL_ACK/TRIGGER_2_RISE_WAIT/TRIGGER_2_RISE_ACK/INPUT_FRAME/INPUT_GATE


Attributes
----------
============================ ======= ======================= ======================================================================
Attribute name		     RW      Type                    Description
============================ ======= ======================= ======================================================================
frame_mode                   rw      DevString               The frame mode, **single** or **fast_transfer**
frame_trigger_signal_type    rw      DevString               The frame trigger signal type (input #1)
sequence_gate_signal_type    rw      DevString               The gate signal type (input #2)
electronic_shutter_enabled   rw      DevString               The electronic shutter **true** or **false** to activate or not
cooler_temperature_setpoint  rw      DevDouble               The cooling system temperature setpoint in Celsuis
sensor_temperature_setpoint  rw      DevDouble               The detector (sensor) temperature setpoint in Celsuis
output1_signal_type          rw      DevString               The output #1 signal type
output2_signal_type          rw      DevString               The output #2 signal type
output1_id                   rw      DevString               The output #1 signal source
output2_id                   rw      DevString               The output #2 signal source
vacuum_valve		     rw      DevString               The vacuum valve command **true** or **false** to open or close
============================ ======= ======================= ======================================================================

**Warning**: be careful with the temperature setting (and vacuum valve), the operating temperature is factory-determined and should never 
be changed. There is no reason to run the detector at a warner temperature.

For the signal type and source the possible values are listed above in the *Properties* section.


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
