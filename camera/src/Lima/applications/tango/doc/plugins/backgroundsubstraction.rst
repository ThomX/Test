Background Substraction
=======================

The Background substraction correction is a simple operation you can active when a detector has some dark-current 
noise independent of the dose of photons it will receive.
To set the correction  you must provide to the device a background image (**setBackgroundImage** cmd) file and 
then start (**start** cmd) the correction.


Properties
----------
This device has no property.

Attributes
----------

======================= ======= ======================= ======================================================================
Attribute name		RW	Type			Description
======================= ======= ======================= ======================================================================
RunLevel		rw	DevLong                 Run level in the processing chain, from 0 to N
delete_dark_after_read  rw      DevBoolean              If true the device will delete the file after reading
                                                        Can be useful to not keep obsolete dark image file after use	
======================= ======= ======================= ======================================================================


Commands
--------

=======================	=============== =======================	===========================================
Command name		Arg. in		Arg. out		Description
=======================	=============== =======================	===========================================
setBackgroundImage      DevString       DevVoid                 Full path of background image file
Start                   DevVoid        	DevVoid			Start the correction for next image
Stop                    DevVoid         DevVoid                 Stop the correction after the next image 
=======================	=============== =======================	===========================================


