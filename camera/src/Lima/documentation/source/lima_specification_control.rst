Control Layer Detailed
----------------------
The common software control layer, providing unique client interface

Overview
````````
The control layer will provide a user-level API to control any detector supported by LIMA. The main role of the control layer is to provide commands and parameters for: 

* accessing main detector characteristics
* accessing detector hardware capabilities
* acquisition sequencing and triggering
* access to image buffers
* file saving operations
* image processing applied on each frames (image reconstruction, flip, rotate, ...)
* image processing to prepare image for online display (compression, additional binning, ...)
* computation of counters like integral applied on a roi, beam position, on each frame or not


in the following section, parameters are listed by group of functionnality. For each parameter, set/get functions will be provided to read/write these parameters. Also for all group of parameters, global set/get functions should allow to read/write them all in one call.

As an extra functionality, functions that export/import the global configuration to/from string (XML) format can be foreseen. However, this feature is not needed in the first implementation of the library, and may be left for future versions. 

Detector informations
`````````````````````
We distinguish two types of detector informations:

* the main detector chararcteristics which are always provided.
* the detector hardware capabilities which are depends on the controller capabilities. 

Detector main characteristics
'''''''''''''''''''''''''''''
These are **read-only** parameters to inform client about the main characteristics of the detector. These informations does not include the detector capabilities: frame transfer mode, kinetic mode, hardware binning and roi, ...

================ =============================================
Parameters 	 Description
================ =============================================
MaxImageSize 	 Maximum size of image
DefaultImageType Default data type of image (ushort, ulong,...)
PixelSize 	 Physical size of pixels
DetectorType 	 Type of the detector (Frelon, Maxipix,...)
DetectorModel 	 Model of the detector
================ =============================================

Detector hardware capabilities
''''''''''''''''''''''''''''''
These hardware capabilities are functionnalities provided or not by a specific controller. They are precisely defined in the hardware layer for each detector. Typical examples are:

* hardware kinetic mode
* frame transfer mode
* ... 

Acquisition
```````````
Different acquisition mode can be defined for all detectors:

* Single: acquisition of one or a sequence of single frames. This is the default mode.
* Accumulation: the final frame is a sum of several acquired sub-frame (to overcome overflow for instance)
* Concatenation: the final frame is a concatenation of several acquired sub-frame 

Other acquisition mode might be possible depending on the hardware : kinetic mode or transfer mode for instance. These modes will then be listed in the hardware capabilities if they exist.

Acquisition Commands
''''''''''''''''''''
To perform acquisition, we define 3 functions:

* **PrepareAcq :** prepare acquisition (software operation registration, buffer allocations,...) in order to minimize the time needed for next StartAcq call
* **StartAcq :** start acquisition
* **StopAcq :** interrupt acquisition 

Acquisition Parameters
''''''''''''''''''''''

+----------------------+---------------------------------------------------------------------+
| **Parameter**	       | **Description**						     |
+----------------------+---------------------------------------------------------------------+
|AcqNbFrames           | Number of frames to acquire.                                        |
|                      | This define an acquisition sequence. If set to 0 endless acquisition|
+----------------------+---------------------------------------------------------------------+
|AcqExposureTime       | Frame exposure time                                                 |
+----------------------+---------------------------------------------------------------------+
|LatencyTime           | Latency time between frames                                         |
+----------------------+---------------------------------------------------------------------+
| AcqFrameMode         | Acquisition frame mode:                                             |
|		       |                                                                     |
|		       | * Single : default standard mode                                    |
|		       | * Accumulation : one frame is an accumulation of multiple frames    |
|		       | * Concatenation : one frame is a concatenation of several frames    |
+----------------------+---------------------------------------------------------------------+
|AccNbFrames	       | *in case of AcqFrameMode==Accumulation only:*                       |
|                      |                                                                     |
|		       | sets the number of accumulated sub-frames.                          | 
|		       | This automatically update AccExposureTime.                          |
+----------------------+---------------------------------------------------------------------+
|AccExposureTime       | in case of AcqFrameMode==Accumulation only:                         |
|                      |                                                                     |
|		       | sets the exposure time for each sub-frame.                          |
|                      | This automatically update NbAccFrames.                              |
+----------------------+---------------------------------------------------------------------+
|ConcatNbFrames	       | in case of AcqFrameMode==Concatenation only                         |
|                      |                                                                     |
|		       | sets the number of concatenated sub-frame to define one ready frame.| 
|		       | The total number of frame to acquire is still set by AcqNbFrames    |
+----------------------+---------------------------------------------------------------------+
|Trigger	       | Triggering mode                                                     |
|                      |                                                                     |
|		       | * Internal : software triggering                                    |
|		       | * ExtStart : one external signal to start the whole sequence        |
|		       |   acquisition (one or more frames per sequence)                     |
|		       | * MultExtStart : one external signal for each frame                 |
|		       |   in the acquisition sequence                                       |
|		       | * Gate : controls start and stop of each frame                      |
|		       | * ExtStartStop : one start signal to start acquisition              |
|		       |   of one frame and one signal to stop it                            |
+----------------------+---------------------------------------------------------------------+
|ExtShutterMode	       | External Shutter mode:                                              |
|                      |                                                                     |
|		       | * Manual                                                            |
|		       | * AutoPerFrame : shutter automatically open and close for each frame|
|		       | * AutoPerSequence : shutter automatically open at                   |
|		       |   the beginning of a sequence and close at the end                  |
+----------------------+---------------------------------------------------------------------+
|ExtShutterOpenTime    | Opening time of the shutter                                         |
+----------------------+---------------------------------------------------------------------+
|ExtShutterCloseTime   | Closing time of the shutter                                         |
+----------------------+---------------------------------------------------------------------+
|ReadoutTime	       | *Read-Only* frame readout time                                      |
+----------------------+---------------------------------------------------------------------+
|FrameRate	       | *Read-Only* frame rate in Hz                                        |
+----------------------+---------------------------------------------------------------------+

Image
`````
The image parameters helps defining image size, buffering methods and basic image operations. The common image operations are executed in following order:

* flip horizontally and / or vertically
* extract the ROI
* perform image binning 

Depending on the controller, hardware flip/binning/roi migth be not possible or constrained to limited values. To overcome these limitations, software operations are provided. Both software and hardware operations can be defined together, hardware operations executed first. The software operations can then perform additionnal computations to fit the user needs. To help users sets both automatically, some wizard functions are provided, specially helpful when mixing hardware and software operations. Even in the case of full wizard mode, the hardware and software operations effectively performed can be read by the user. 

Image Parameters
''''''''''''''''

+---------------+-------------------------------------------------------------------------------+
|Parameters	| Description									|
+---------------+-------------------------------------------------------------------------------+
|MaxImageSize	| Read-Only Depending on the acquisition parameters set, this gives the maximum	|
|		| image size which can be acquired						|
+---------------+-------------------------------------------------------------------------------+
|ImageSize	| Size of image which can be read						|
+---------------+-------------------------------------------------------------------------------+
|ImageType	| Numerical type of image (ushort, ulong,...)					|
+---------------+-------------------------------------------------------------------------------+
|BufferMode	| Image buffer mode can be:							|
|		|										|
|		| * Linear									|
|		| * Circular									|
+---------------+-------------------------------------------------------------------------------+
|NbBuffers	| Number of buffers to be used. If set to 0, automatic allocation		|
|		| of buffers is done								|
+---------------+-------------------------------------------------------------------------------+
|MaxMemory	| Percentage of available memory that buffers can used.				|
|		| If <NbBuffers> is set to 0, this amount of memory will be used.		|
+---------------+-------------------------------------------------------------------------------+
|HardBinning	| Defines Vertical and Horizontal Binning performed by hardware			|
+---------------+-------------------------------------------------------------------------------+
|HardRoi	| Defines ROI in Binned units performed by hardware				|
+---------------+-------------------------------------------------------------------------------+
|HardFlip	| Horizontal and/or Vertical flip performed by hardware				|
+---------------+-------------------------------------------------------------------------------+
|SoftBinning	| Defines Vertical and Horizontal Binning performed by software			|
+---------------+-------------------------------------------------------------------------------+
|SoftRoi	| Defines ROI in Binned units performed by software				|
+---------------+-------------------------------------------------------------------------------+
|SoftFlip	| Horizontal and/or Vertical flip performed by software				|
+---------------+-------------------------------------------------------------------------------+
|RoiBinOffset	| To allow binning not aligned on first row/column				|
+---------------+-------------------------------------------------------------------------------+
|WizardMode	| WizardMode defines the way roi and binnings are computed when			|
|		| defined by the wizard functions:						|
|		|										|
|		| * HardwareOnly								|
|		| * SoftwareOnly								|
|		| * HardwareAndSoftware : perform as much as possible with hardware and		|
|		|   complete with software if needed						|
+---------------+-------------------------------------------------------------------------------+
|WizardBinning	| Defines HardBinning and/or SoftBinning depending on WizardMode		|
+---------------+-------------------------------------------------------------------------------+
|WizardRoi	| Defines HardRoi and/or SoftRoi depending on WizardMode			|
+---------------+-------------------------------------------------------------------------------+
|WizardFlip	| Defines HardFlip and/or SoftFlip depending on WizardMode			|
+---------------+-------------------------------------------------------------------------------+

File Saving
```````````
There are basically two main modes to save image file. Either it can be done on demand or automatically. If saving is automatic, we let the choice to do it when frame is ready or when the frame header has been set. Automatic save operation will be added at the end of all frame software operations. In case of manual mode, the write call is not blocking, It is the client's responsibility to ask if the frame saving has finished.
File saving commands

* **SaveImage(FrameNumber):** save the image corresponding to the Frame number or -1 to save the last available frame 

File saving header
''''''''''''''''''
The header is composed of three parts:

* **Static header :** contains enough information to read the file
* **Common header :** can be set or not for all frame in the sequence
* **Frame header :** can be set or not for one specific frame 

File saving parameters
''''''''''''''''''''''

Filename is created as follow:
Directory/Prefix_Number.Suffix

+---------------+-----------------------------------------------+
| **Parameters**| **Description**				|
+---------------+-----------------------------------------------+
|Directory	| Directory to save file			|
+---------------+-----------------------------------------------+
|Prefix		| Filename prefix				|
+---------------+-----------------------------------------------+
|Suffix		| Filename suffix				|
+---------------+-----------------------------------------------+
|Number		| Next file number				|
+---------------+-----------------------------------------------+
|Format		| EDF,RAW and CBF for now. Others may come	|
+---------------+-----------------------------------------------+
|OverWritePolicy| Policy to apply when file exist		|
|		|						|
|		| * Abort : to not save anything if file exist	|
|		| * Overwrite : overwrite the existing file	|
|		| * Append : if file already exist,		| 
|		|   append new image to that file		|
+---------------+-----------------------------------------------+
|Mode		| Allowed saving modes are:			|
|		|						|
|		| * Manual : saving is executed			|
|		|   on user call (default)			|
|		| * AutoFrame : automatic saving		|
|		|   when frame is ready				|
|		| * AutoHeader: automatic saving when frame	|
|		|   is ready and frame header has been received |
+---------------+-----------------------------------------------+
|FramesPerFile	| Number of frame saved per file		|
+---------------+-----------------------------------------------+
|StaticHeader	| Read-Only informations automatically write	|
|		| for all files					|
+---------------+-----------------------------------------------+
|CommonHeader	| Header which will be appended after		|
|		| StaticHeader for all frame in the sequence	|
+---------------+-----------------------------------------------+

Software Operations
```````````````````
All operations performed on acquired frames by the lima control layer are called software operations. This includes image transformations like software binning or roi but also image saving for example. These operations might be requested either internally by the control layer code itself, or by the external user. We differentiate 4 types of operations:

* image operations : operations on each image in a sequencial order which produces a transformed image
* counter operations : operations performed on each image without any image output
* monitor operations : same as counter operations but not performed on each frame
* visualisation operations  : specific image transformation for visualisation purpose 

Image Operations
''''''''''''''''
These operations are executed on each frame and sequentially. Each operations output a new transformed image which will be input of next operation. Typically this can be sofware binning, roi or software image reconstruction. An image will be considered ready when all image operations has been performed on it. All image operation are executed sequencially for one frame. The operations are parallelized for several frames. Basic image operations (transformations):

* Region of Interest (RoI)
* Binning
* Vertical/Horizontal Flip
* Rotate
* Background subtraction
* Flat Field normalization
* Distortion correction 

We differentiate 2 types of image operations:

* INTERNAL : these operations has been added by the library itself. They are executed before all external operations and are performed in-place (on the hardware buffer). Once all the internal image operations has been performed we consider that the **base image** is ready. We have 4 operations of this kind executed in the following order:
  - image reconstruction : this one might be added by the hardware layer.
  - image flip : added by the control library
  - image binning : added by the control library
  - image roi : added by the control library 

* EXTERNAL : these operations are added by the external user. Input data of first operation is taken from the hardware buffer and then all output images are stored in an auxilliary buffer called ImageOperationList. Once all external image operations has been performed, we consider the **image ready**. An additionnal RUNLEVEL parameter will give the order in which image operations will be executed. 

Image commands:

* ReadImage( F# | -1 ) : reads a specific frame or the last one available from the ImageOperationList buffer.
* ReadBaseImage( F# | -1 ) : reads a specific frame or the last one available from the hardware buffer. 

Counter Operations
''''''''''''''''''
These operations takes an image as input and output one or several values per images. They are executed on each frame once at list all internal operations has been performed. They are added at a specific RUNLEVEL of the image operations and are executed in a parallel way. A specific buffer keeps an history of computed values for each counter. Once all counters specified on a frame has been computed, the counter ready flag is set. Counter examples:

* BPM (requires an additional buffer)
* RoI counters [Integral, Average, Standard Deviation]
* Treshold counters
* RoI concatenation buffer (tomography sinogram) 

There is one specific counter operation AutoSave which is internally set by the library at the end of all image operations. This operation handle all saving operations.

Counter commands:

* ReadCounters( F# | -1 )
* ReadCountersHistory( FromF#, ToF# | -1 ) 

Monitor Operations
''''''''''''''''''
These are similar to the counter operations, except that they are not applied on each frame. Also, no history is kept for monitors : only the last computed monitor is available. If several monitors are used, the last frame number on which each one has been computed can be different from one monitor to the other. Two sampling modes can be defined independantly for each monitor:

* *as-fast-as-possible:* the update frequency is defined by the PC load
* *periodic:* the operation is executed every N frames. 

Monitor commands:

* ReadMonitors() --> [F#1, Monitor1Value], ... , [F#N, MonitorNValue]
* ReadMonitorsStatus() --> [F#1, ..., F#N] 

Visualisation Operations
''''''''''''''''''''''''
The goal of these operations is to give the user a real time visualization of the image, but it does not need to be at full speed and/or resolution. Only one image is kept in memory. Configuration parameters could be:

* display binning
* spatial/temporal compression
* copy image in a shared memory 

Monitor commands:

* ReadDisplayImage() --> Last image available 

Status
``````
The acquisition, saving and software processing status can be retrieved by a single function : **GetStatus**

The parameters read from this function are:

* *DetectorStatus:* reflects the status of the detector:

  - Ready
  - Exposure
  - Shutter open/close
  - Image transfer (in Frame Transfer Mode)
  - Image readout
  - Inter-frame sleep stage 

* *LastImageAcquired:* last frame number readout and buffered. The frame might not yet be ready for client reading.
* *LastBaseImageReady:* last frame number readout, buffered and on which all internal operations has been done.
* *LastImageReady:* last frame number which is ready for client reading. This means the frame has been readout and buffered, and all eventual software operations has been performed on that frame
* *LastImageSaved:* last frame number which has been saved.
* *LastImageCounter:* last frame number on which all optional counters has been computed. 

Software Operations and Status flow chart
`````````````````````````````````````````
.. image:: lima_specification/LIMAControlLayer_flowchart.png
