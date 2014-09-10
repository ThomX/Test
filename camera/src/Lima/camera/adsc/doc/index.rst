.. _camera-adsc:

Adsc camera plugin
-------------------

.. image:: adsc.jpg

Introduction
```````````
ADSC : Area Detector System Corporation

The Lima module has been tested only with the 315r model:

Prerequisite
````````````
2 programs have to be running on the ADSC server: ccd_image_gather and det_api_workstation

Initialisation and Capabilities
````````````````````````````````
In order to help people to understand how the camera plugin has been implemented in LImA this section
provide some important information about the developer's choices.

Camera initialisation
......................

Here are the available functions

* SetHeaderParameters
* UseStoredDarkImage
* SetImageKind
* SetLastImage

Std capabilites
................

This plugin has been implemented in respect of the mandatory capabilites but with some limitations according 
to some programmer's  choices.  We only provide here extra information for a better understanding
of the capabilities for the Adsc camera.

* HwDetInfo

	- Max image size is : 3072 * 3072
	- 16 bit unsigned type is supported

* HwSync

  trigger type supported are:
	- IntTrig
	
  
Optional capabilites
........................

* HwBin
	- 1 * 1
	- 2 * 2

	
Configuration
``````````````
No Specific hardware configuration are needed

How to use
````````````
here is the list of accessible fonctions to configure and use the ADSC detector:

.. code-block:: cpp

	void 	setHeaderParameters(const std::string& header);	
	void	setStoredImageDark(bool value);
	bool	getStoredImageDark(void);
	void    setImageKind(int image_kind);
	int	    getImageKind(void);	
	void    setLastImage(int last_image);
	int	    getLastImage(void);	

	void                setFileName(const std::string& name);
	const std::string&  getFileName(void);
	void                setImagePath(const std::string& path);
	const std::string&  getImagePath(void);
