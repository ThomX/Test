Maxipix Tango device
=======

This is the reference documentation of the Maxipix Tango device.

you can also find some useful information about the camera models/prerequisite/installation/configuration/compilation in the :ref:`Maxipix camera plugin <camera-maxipix>` section.


Properties
----------

===================== =============== =============== ==============================================================
Property name	      Mandatory	      Default value   Description
===================== =============== =============== ==============================================================
config_name	      Yes             N/A             The configuration name
config_path	      Yes             N/A             The configuration directory path where the files are available
espia_dev_nb	      No              0	              The acquisition Espia board number
reconstruction_active No              True            Activate the reconstruction or not
fill_mode	      No              Raw             the chip-gap filling mode, **Raw**, **Zero**,
                                                       **Dispatch** or **Mean**.
gate_level	      No              High_Rise       The Input gate level, **High_rise** or
						      **Low_Fall**
gate_mode	      No              Inactive        The gate mode, **Inactive** or **Active**
ready_level	      No              High_Rise       The output ready level, **High_rise** or
                                                      **Low_Fall**
ready_mode	      No              Exposure        The output Ready mode, **Exposure** or
                                                      **Exposure_Readout**
shutter_level	      No              High_Rise       The output Shutter level, **High_rise** or **Low_Fall**
trigger_level	      No              High_Rise       The output Trigger level, **High_rise** or **Low_Fall**
===================== =============== =============== ==============================================================


Attributes
----------
======================= ======= ======================= ===========================================================
Attribute name		RW	Type			Description
======================= ======= ======================= ===========================================================
config_name		rw	DevString		the configuration name. If changed the detector is
							reconfigured and reset.
config_path		rw	DevString		the configuration directory path where the files are
							available
energy_calibration	rw	Spectrum		The energy calibration, [0] = threshold setpoint , [1]
				DevDouble		threshold step-size (keV)
energy_threshold	rw	DevDouble		The threshold in energy (keV)
threshold		rw	DevDouble		The detector threshold
threshold_noise		rw	Spectrum		The threshold noise of each chip, [0] =chip0 thl, [0] =
				DevDouble		chip1 thl, ....
espia_dev_nb		rw	DevString		The Espia board number.
fill_mode		rw	DevString		The chip-gap filling mode:
							 - **Raw**, the border pixel values are copied
							 - **Zero**, border and gap pixel are set to zero
							 - **Dispatch**, the border pixel values are
							   interpolated over the full gap
							 - **Mean**, the gap pixels are filled with the border
							   pixels average value.
gate_level		rw	DevString		The Input gate level:
							 - **High_rise**
							 - **Low_Fall**
gate_mode		rw	DevString		The gate mode:
							 - **Inactive**
							 - **Active**
ready_mode		rw	DevString		The output Ready mode:
							 - **Exposure**
							 - **Exposure_Readout**

shutter_level		rw	DevString		The output Shutter level
							 - **High_rise**
							 - **Low_Fall**
trigger_level		rw	DevString		The output Trigger level:
							 - **High_rise**
							 - **Low_Fall**
dac_possible            ro      DevString[]             Return the list of the the possible dac names
dac_name                rw      DevString               The dac name to be write/read (dac_value)
dac_value               rw      DevLong                 The dac value of the given dac_name dac register
======================= ======= ======================= ===========================================================

**Warning**: we recommend to not change the DAC register values (dac_name and dac_value attributes) excepted
if you well know what you are doing, if you have some troubles with the detector please contact the ESRF Detector
Unit first.
 
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

