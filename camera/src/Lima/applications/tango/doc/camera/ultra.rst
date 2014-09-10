Ultra Tango device
=======

This is the reference documentation of the Ultra Tango device.

you can also find some useful information about the camera models/prerequisite/installation/configuration/compilation in the :ref:`Ultra camera plugin <camera-ultra>` section.

Properties
----------

=============== =============== =============== =========================================================================
Property name	Mandatory	Default value	Description
=============== =============== =============== =========================================================================
headIpaddress	No		192.168.1.100   The detector head IP address	
hostIpaddress   No              192.168.1.103   The host IP address
tcpPort         No              7               The tcp echo port
udpPort         No              5005            The upd port
nPixels         No              512             The number of detector pixels
=============== =============== =============== =========================================================================


Attributes
----------
======================= ======= ======================= ======================================================================
Attribute name		RW	Type			Description
======================= ======= ======================= ======================================================================
headColdTemp            ro      DevFloat                The head cold temperature in K
heatHotTemp             ro      DevFloat                The head hot temperature in K
tecColdTemp             ro      DevFloat
tecSupplyVolts          ro      DevFloat
adcPosSupplyVolts       ro      DevFloat
adcNegSupplyVolts       ro      DevFloat
vinPosSupplyVolts       ro      DevFloat
vinNegSupplyVlots       ro      DevFloat
headADCVdd              ro      DevFloat
headVdd                 rw      DevFloat
headVref                rw      DevFloat
headVrefc               rw      DevFloat
headVpupref             rw      DevFloat
headVclamp              rw      DevFloat
headVres1               rw      DevFloat
headVres2               rw      DevFloat
headVTrip               rw      DevFloat
fpgaXchipReg            rw      DevULong
fpgaPwrReg              rw      DevULong
fpgaSyncReg             rw      DevULong
fpgaAdcReg              rw      DevULong
frameCount              ro      DevULong
frameError              ro      DevULong
headPowerEnabled        rw      DevBoolean
tecPowerEnabled         rw      Devboolean
biasEnabled             rw      Devboolean
syncEnabled             rw      Devboolean
calibEnabled            rw      Devboolean
8pCEnabled              ro      DevBoolean
tecOverTemp             ro      DevBoolean
adcOffset               rw      DevFloat[16]
adcGain                 rw      DevFloat[16]
aux1                    rw      DevULong[2]
aux2                    rw      DevULong[2]
xchipTiming             rw      DevULong[9]
======================= ======= ======================= ======================================================================

Please refer to the manufacturer's documentation for more information about the above listed parameters and how to use them.

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
SaveConfiguration       DevVoid         DevVoid                 Save the current configuration
RestoreConfiguration    DevVoid         DevVoid                 Restore the latest configuration
=======================	=============== =======================	===========================================
