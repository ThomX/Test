LimaTacoCCD
============

This device has been created by legacy and it provides the only interface that SPEC software is supporting for "ESRF General CCD Dev"  CCD-like
controller.

Properties
----------

======================= =============== =============== =========================================================================
Property name	        Mandatory	Default value	Description
======================= =============== =============== =========================================================================
ManualAsynchronousWrite No		False           Flag for manual writting, can improve the performance of data saving
======================= =============== =============== =========================================================================

Attributes
----------
This device has no attributes.

Commands
--------

=======================	==================== ======================= ===========================================
Command name		Arg. in		     Arg. out                Description
=======================	==================== ======================= ===========================================
TacoState		DevVoid		     DevLong                 Return the device taco-like state
DevCcdStart             DevVoid		     DevVoid                 Start the acquisition
DevCcdStop              DevVoid		     DevVoid		     Stop the acquisition
DevCcdRead              DevVarLongArray[2]:  DevVarCharArray:        Return the image as a string
                        frame_nb,frame_size  the raw image
DevCcdReadAll           DevLong:             DevEncoded              Return the concatenated frames in a DevEncoded
                                                                     format DATA_ARRAY (see  :ref:`data_array_encoded`)
                        frame_size
DevCcdReadJPeg          DevShort:
                        jpeg compression     DevVarCharArray:        Return a jpeg image
                                             Jpeg image
DevCcdWrite             DevVoid              DevVoid                 Save the last image
DevCcdSetExposure       DevFloat             DevVoid                 Set the exposure time in second
DevCcdGetExposure       DevVoid              DevFloat                Return the exposure time in second
DevCcdSetRoI            DevVarLongArray[4]:  DevVoid                 Set the new Region-of-Interest
                        startx,endx,starty,
                        endy
DevCcdGetRoi		DevVoid              DevVarLongArray[4]:     Return the last Region-of-Interest
                                             startx,endx,starty,
 					     endy
DevCcdSetFilePar       DevStringArray[5]
DevCcdHeader
DevCcdImageHeader
DevCcdHeaderDelimiter
DevCcdGetFilePar
DevCcdDepth
DevCcdYSize
DevCcdXSize
DevCcdReset
DevCcdSetMode
DevCcdGetMode
DevCcdWriteFile
DevCcdGetBin
DevCcdSetBin
DevCcdSetFrames
DevCcdGetFrames
DevCcdSetTrigger
DevCcdGetTrigger
DevCcdReadValues
DevCcdSigValues
DevCcdGetLstErrMsg
DevCcdGetCurrent
DevGetDebugFlags
DevSetDebugFlags
=======================	=============== =======================	===========================================



