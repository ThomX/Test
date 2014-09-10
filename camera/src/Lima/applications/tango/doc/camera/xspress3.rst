Xspress3 Tango device
=========

This is the reference documentation of the Xspress3 Tango device.

you can also find some useful information about the camera models/prerequisite/installation/configuration/compilation in the :ref:`Xspress3 camera plugin <camera-xspress3>` section.


test reference to camera plugin section: :ref:`camera-adsc`

Properties
----------

================= =============== =============== =========================================================================
Property name	  Mandatory       Default value   Description
================= =============== =============== =========================================================================
basIpaddress      No		  none            Override the base IP address (e.g. 192.168.0.1)
                                                  from which all other addresses are calculated or NULL to use the default
basMacAddress     No              none            Override the base MAC address (e.g. 02.00.00.00.00)
                                                  from which all other card MAC address`s are calculated or NULL to use the default
basePort          No              none            Override the base IP port number or 0 to use the default
createScopeModule No              False           true = do not create a scope data module
nbFrames          No              1               Number of 4096 energy bin spectra timeframes
scopeModName      No              NULL            The scope data module filename or NULL to use the default
nbCards           No              1               The number of xspress3 cards that constitute the xspress3 system,
                                                  between 1 and  XSP3_MAX_CARDS
nbChans           No              -1              Limit the number of channels
debug             No              0               debug message (0 = off, 1=normal, 2=verbose)
noUDP             No              False           True = do not do UDP connection
cardIndex         No              none            Starting card index
directoryName     No              non             The directory name to save and restore configurations
================= =============== =============== =========================================================================



Attributes
----------
======================= ======= ======================= ======================================================================
Attribute name		RW	Type			Description
======================= ======= ======================= ======================================================================
card			rw	DevLong
numChan                 ro      DevLong
numCards                ro      DevLong
chansPerCard            ro      DevLong
maxNumChan              ro      DevLong
binsPerMca              ro      DevLong
windows                 rw      DevLong[32]
runMode                 rw      DevBoolean[4]
clocks                  rw      Devbooleanp[3]
goodsThreshold          rw      DevLong[16]
dtcEnergy               rw      DevDouble
dtcParameters           rw      DevDouble[48]
scaling                 rw      DevDouble[8]
fanTemperatures         rw      DevDouble[50]
fanController           rw      DevDouble[2]
setPoint                wo      DevDouble
roi                     wo      DevLong[25]
useDtc                  rw      DevBoolean
setTiming               wo      DevLong
adcTempLimit            wo      DevLong
setPlayback             wo      DevBoolean
playbackfilename        wo      DevString
dataSource              rw      DevLong[8]	
======================= ======= ======================= ======================================================================

Commands
--------

=======================	================ ======================= ===========================================
Command name		Arg. in		 Arg. out		 Description
=======================	================ ======================= ===========================================
Init			DevVoid 	 DevVoid                 Do not use
State			DevVoid		 DevLong                 Return the device state
Status			DevVoid		 DevString               Return the device state as a string
getAttrStringValueList	DevString:	 DevVarStringArray:      Return the authorized string value list for
			Attribute name	 String value list       a given attribute name
Reset                   DevVoid          DevVoid
InitBrams               DevLong:         DevVoid
                        channel
Pause                   DevVoid          DevVoid
Restart                 DevVoid          DevVoid
Arm                     DevVoid          DevVoid
Clear                   DevVoid          DevVoid
SaveSettings            DevVoid          DevVoid
RestoreSettings         DevBoolean       DevVoid                 Force restore if major revision of saved file
                                                                 does not match the firmware revision
InitRois                DevLong:         DevVoid                 
                        channel
ReadHistogram           DevVarLongArray: DevVarULongArray:       Return the histogram data
                        frame, channel         
ReadScalers             DevVarLongArray: DevVarULongArray:       Return the scaler data
                        frame, channel
StartScope              DevVoid          DevVoid
LoadPlayback            DevVarLongArray: DevVoid                                  
                        src0,src1,
                        [num_streams,
                        digital]
FormatRun               DevVarLongArray: DevVoid
                        chan,[nbits_eng,
                        aux1_mode,
                        adc_bits,
			min_samples,
			aux2_mode,
			pileup_reject
=======================	================ ======================= ===========================================
