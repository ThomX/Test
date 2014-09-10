Hardware Layer Detailed
-----------------------

The hardware interface implementing specific detector control 

Overview
````````

The Hardware Layer defines the interface between the Control Layer and the controller library. It provides the minimal functionality needed for the Control Layer to satisfy the user requests. Requests coming from the Hardware Layer have already checked for consistency, so the Hardware Layer must focus only in the control of the hardware.

The main class in the Hardware Layer is the HwInterface, providing the interface to the Control Layer. In order to provide a flexible and evolvable interface, the configuration of this layer is implemented as a set of features (capabilities), that may or may not be implemented by the hardware. The capabilities can be grouped in three categories:

* Basic: includes the synchronization parameters (exposure time, ext. trigger, etc), the image buffer management and the frame callbaks, is considered standard and must be implemented for all the detectors.
* Extended: optional common features like image transformations (binning, RoI, flip), advanced acquisition modes (kinetics, frame transfer), and extended mechanisms (camera serial line)
* Specific: these are detector-specific features that can not be treated in a generic interface 

Hardware Interface
``````````````````
As an interface to the Control Layer, this class exports the capabilities provided by the hardware:

* getCapList() 

It also contains the high level commands to control the acquisition:

* reset(reset_level)
* prepare()
* start()
* stop()
* getStatus()
* getNbAcquiredFrames()
* getStartTimeStamp() 

Two reset levels are foreseen:

* SoftReset: any active acquisition is stopped, and all the parameters are set to their default values
* HardReset: like SoftReset, but before setting the default parameters, the hardware is completely reinitialized 

The getStatus() function shoud return the following information:

+---------------+-----------------------------------------------+
| **Parameter** | **Description**				|
+---------------+-----------------------------------------------+
|AcqStatus	| Global acquisition status: Ready/Running/Fault|
+---------------+-----------------------------------------------+
|DetStatus	| Compound bit flags specifying the		|
|		| current detector status:			|
|		|						|
|		| * Fault					|
|		| * WaitForTrigger				|
|		| * ShutterOpen					|
|		| * Exposure					|
|		| * ShutterClose				|
|		| * ChargeShift					|
|		| * Readout					|
+---------------+-----------------------------------------------+
|DetStatusMask	| A mask specifying the detector status		|
|		| bits that are supported by the hardware	|
+---------------+-----------------------------------------------+

.. image:: lima_specification/HW_img.jpg


Basic (Standard) Capabilities
`````````````````````````````
These capabilities are mandatory for all the detectors. They define the minimum functionality necessary for image acquisition.

Detector Information
''''''''''''''''''''

This capability return static information about the detector and the current image dimension.

===============	===========================================================================================
**Parameters** 	**Description**
===============	===========================================================================================
MaxImageSize 	Maximum size of the image
DefImageType 	Default data type of image (ushort, ulong, ...)
CurrImageSize 	Current maximum size of the image, excluding binning and RoI. This is a read-only parameter
CurrImageType 	Data type of image (ushort, ulong, ...)
PixelSize 	Physical size of pixels (in micrometre)
DetectorType 	Type of the detector (Frelon, Maxipix, ...)
DetectorModel 	Model of the detector
===============	===========================================================================================

Synchronization
'''''''''''''''

These parameters are defined in the Control Layer Acquisition Parameters section, and are passed to the Hardware Layer after consistency checks:

======================= ========================================================================================================
Parameter		Description
======================= ========================================================================================================
ExtTrigger		This defines how the image acquisition is started
ExposureTime	 	The software (internal) exposure time, when it is not defined by an external signal; must be 0 otherwise
LatencyTime		Delay (sleep) time between frames when sequence is (internally) controlled by the detector
Shutter Mode		The shutter mode (manual or automatic)
Shutter open/close time Time(s) to open and close the shutter
NbFrames		Number of frames to be taken in the acquisition (sequence) 
======================= ========================================================================================================

Buffer Management
'''''''''''''''''

This block controls the image memory buffer allocation and management. They are used:

* As temporary frame storage before saving, allowing disk/network speed fluctuations.
* To permanently hold images that can be read by the user after the acquisition is finished. 

By default, a buffer holds one frame. However, when the frame size is very small (detector stripes), the individual read of many frames can represent a performance issue. In this case, it is desirable to concatenate frames in a single buffer to be read in one shot.

In the same way, a single output frame can be the result of a software accumulation of several detector frames. Such functionality may be needed to avoid pixel saturation in case of a limited detector dynamic range. Frame accumulation and stripe concatenation are exclusive buffer operations; only one of them can be active at a time.

These buffer functionalities may be implemented by the hardware layer (kernel driver in the case of the Espia). If not, an auxiliary buffer manager class will be provided to facilitate (and unify) its software implementation.

The buffer management parameters are:

=============== ===============
**Parameter** 	**Description**
=============== ===============
NbBuffers 	Number of image buffers in memory.
NbConcatFrames 	The number of concatenated frames per buffer.
NbAccFrames 	The number of detector frames to accumulate into a single buffer.
MaxNbBuffers 	This Read-Only parameter indicates the maximum number of buffers 
                that can be allocated, given the size of the frame and the number of (concatenated) frames per buffer.
BufferMode 	Buffer filling mode (linear or circular)
=============== ===============

The buffer manager must also provide the following methods:

* getBufferPtr(int buffer_nb)
* getFramePtr(int acq_frame_nb)
* getFrameTimeStamp(int acq_frame_nb)

Frame callbacks
'''''''''''''''

The harware must provide callbacks after each acquired frame. The callback function should receive the following information:

=============== ===============
**Parameter** 	**Description**
=============== ===============
AcqFrameNb 	Index of the frame since the start of the acquisition
FramePtr 	Pointer to the frame memory
FrameDim 	Structure holding the width, height and type of the frame
TimeStamp 	Time (in sec.) since the start of the acquisition
=============== ===============

The frame callbacks are implemented by means of an auxiliary class (FrameCallback), which will be used by the Control Layer. From the Hardware Layer point of view, the standard capability control object must implement two functions:

* setFrameCallbackActive(bool cb_active)
* frameReady(<callback_frame_info>)

Extended Capabilities
`````````````````````

The following capabilities are optional, depending on the functionality provided by the hardware. 

Image Operations
''''''''''''''''

This is a group of capabilities affecting the image generated by the dectector. The implementation of each of them is optional.

The order in which the hardware applies the operations is detector-specific. However, the Control Layer will consider that they are applied in the order described here. As a consequence, the parameters of one operation will depend on the previous operations, if they implemented.

Flip
....

=============== ===============
**Parameter** 	**Description**
=============== ===============
FlipHorz 	Flip the image horizontally
FlipVert 	Flip the image vertically
=============== ===============

Binning
.......

=============== ===============
**Parameter** 	**Description**
=============== ===============
BinHorz 	Number of pixels binned in the horizontal (X) direction
BinVert 	Number of pixels binned in the vertical (Y) direction
=============== ===============

Region of Interest (RoI)
........................

The parameters of the RoI are in flipped, binned units.

=============== ===============
**Parameter** 	**Description**
=============== ===============
Origin		X,Y coordinates of the RoI top-left corner (origin)
Size		Width and height of the RoI
=============== ===============

RoIBinOffset
............

If the RoI and Binning capabilities are present, the RoI corners must be aligned to binning units. This optional capability allows to specify the RoI origin in chip pixels, as an offset (modulo) relative to the binning alignment.

=============== ===============
Parameter 	Description
=============== ===============
XOffset 	Real RoI origin X (horizontal) coordinate binning offset
YOffset 	Real RoI origin Y (vertical) coordinate binning offset 
=============== ===============

**Example:**

.. code-block:: c

  Bin = (4,4)
  Roi = (11,15)x(23,47) ->  TopLeft = (44,60), BottomRight = (135,247)
  RoiBinOffset = (1,3)  ->  TopLeft = (45,63), BottomRight = (136,250)

Kinetics
........

The hardware can implement the Kinetics mode, in which a stripe of the CCD is exposed (illuminated window), and after that, the charges are shifted very fast to allow succesive exposures on the same illuminated area. This process is repeated until all pixels of the CCD hold charges from previous exposures. Then, the shutter is closed and a slower readout of the CCD chip is performed, obtaining all the frames corresponding to the different exposures of the illuminated stripe (window), concatenated in a single image.

The associated parameters are:

=============== ===============
**Parameter** 	**Description**
=============== ===============
Mode		Detector specific mode defining the timing
LineBegin 	First CCD line (row) of the illuminated window
WinSize 	Window (stripe) size in pixels
NbStripes 	Number of stripes (exposures) to acquire
=============== ===============

Frame Transfer Mode
...................

Several CCDs provide the Frame Transfer Mode, in which half of the CCD surface is exposed (illuminated) and after that, the charge is shifted to the other half of the chip for readout. While the readout takes place, the illuminated area can start the exposure of the next frame, reducing the detector dead time to the charge shift time.

======================= ====================================
**Parameter**		**Description**
======================= ====================================
FrameTransferMode 	Flag specifying if the FTM is active
======================= ====================================


Timming
.......

It is always desirable to know before the acquisition the detector readout time, and the final frame rate for a given configuration. If such calculation can be done, the hardware implementation should provide this capability.

The following read-only parameters are required:

=============== ========================================================
Parameter 	Description
=============== ========================================================
ReadoutTime 	Time necessary to read a frame (in sec)
FrameRate 	Number of frames per second for the given configuration, 
		including frame transfer mode, exposure time, latency 
		time and shutter open/close time
=============== ========================================================



Serial Line
...........

Some detectors control the hardware through a serial line channel. This optional feature will allow the user to directly talk to the controller code, bypassing the Control and Hardware Layers. Such mechanism is useful in hardware and software diagnostics.

======================= ====================================
Parameter		Description
======================= ====================================
SerialLineTimeout 	Timeout (in sec) for read operations
======================= ====================================

* serialTimeout 

The implementation should provide the following functions:

* serialWrite(char \*wbuffer, int wsize)
* serialRead(char \*rbuffer, int& rsize, bool no_block=false)
* serialReadLine(char \*rbuffer, int& rsize)
* serialWriteRead(char \*wbuffer, int wsize, char \*rbuffer, int& rsize)
* serialWriteReadLine(char \*wbuffer, int wsize, char \*rbuffer, int& rsize) 

To efficiently manage no-blocking operations, serial line callbacks are also defined, with the same philosophy of the frame callbacks. An auxiliary class, SerialLineCallback will be defined, and the following methods must be provided and used by the hardware implementation:

* setSerialLineCallbackActive(bool active)
* serialReadCharAvailable() 

Detector-specific Capabilities
..............................

The access to detector-specific parameters is also provided through one or more non-standard capabilities. These capabilities are not used by the Control Layer, but they exported to the library client, who must know their particular interfaces. Typical examples include the Maxipix specific configuration parameters like the common low level threshold value and its per-pixel complementary matrix, or the FReLoN HD high speed/resolution configuration mode. 
