// *************** imagestar4022control.h *******************


#ifndef _PSL_VHR_H_
#define _PSL_VHR_H_

#define i32 signed long
#define ui32 unsigned long

#define MICROSECONDS 0
#define MILLISECONDS 1
#define SECONDS 2

/* FUNCTION PROTOTYPES
 * -------------------------------------------
 */

#ifdef MAKE_A_DLL
#define LINKDLL __declspec(dllexport)
#else
#define LINKDLL __declspec(dllimport)
#endif



#ifdef __cplusplus
extern "C" {
#endif








/////////////////////////// initialise camera system /////////////////////////////////////////////////////////////////////
LINKDLL 	int		PSL_VHR_Init(char * PSL_camera_files_path); 
// initialise 4022 camera system

LINKDLL		char	*PSL_VHR_get_camera_identifier();
// looks in the camera files directory for 'identifier.txt', reads and returns
// a pointer to an identifier string, or NULL if file was not found

LINKDLL 	bool	PSL_VHR_ReadMaxImageWidth(int  *pMaxWidth);
// reads maximum possible unbinned width of which camera is capable
LINKDLL 	bool	PSL_VHR_ReadMaxImageHeight(int  *pMaxHeight);
// reads maximum possible unbinned height of which camera is capable




//////////////////////////// load appropriate files to support flat fielding and remapping ///////////////////////////////
LINKDLL bool PSL_VHR_load_flat_field_file(char *flatfile);
// loads specified flat field file, usually 'flat.flf' can be specified

LINKDLL bool PSL_VHR_open_map(char *mapfile);
// loads specified remapping file, usually 'distortion.map' can be specified. 
// Return true if specified file was found and was of correct type as identified by header





///////// specify which corrections should be applied to acquired image using PSL_VHR_apply_post_snap_processing /////////
LINKDLL bool PSL_VHR_enable_offset_subtraction(bool bReadoutSubtractionEnabled);	// offset correction
LINKDLL bool PSL_VHR_enable_dark_field_subtraction(bool bDarkSubtractionEnabled);	// bright pixel
LINKDLL bool PSL_VHR_enable_flat_field_subtraction(bool bFlatSubtractionEnabled);	// flat field
LINKDLL void PSL_VHR_enable_noise_reduction(bool bEnabled);							// noise reduction
LINKDLL void PSL_VHR_enable_sharpening(bool bEnabled);								// sharpening




/////////////////// configure virtual shutter parameters /////////////////////
LINKDLL bool PSL_VHR_set_shutter_parameters(int iShutterStartOffset,int iShutterEndOffset,int iShutterPedestal);
 // 1/. first use this to set virtual shutter pedestal e.g. to set pedestal to 1000, 
 // use PSL_VHR_set_shutter_parameters (1,99,1000) . Just use 1 and 99 for first two parameters - these are not used.
LINKDLL void PSL_VHR_set_virtual_shutter_mode(int vshutmode); 
 // 2/. then use this to set virtual shutter mode: 0= FASTER 1=BETTER QUALITY 2=NONE
LINKDLL bool PSL_VHR_enable_virtual_shuttering(bool bVshutterEnabled);
 // 3/. finally use this to enable/disable virtual shutter






//////////////////////////// camera control functions /////////////////////////////////////
LINKDLL 	bool	PSL_VHR_Set_subarea_and_binning(int left,int top,int right,int bottom,int xbin,int ybin);

LINKDLL 	bool	PSL_VHR_WriteExposure(int iExpTime);
// sets integration time in milliseconds

LINKDLL 	bool	PSL_VHR_Set_trigger_mode(int iTrigMode);
// 1 = software triggered
// 2 = h/w trigger rising
// 3 = h/w trigger falling
// 4 = free running
// 5 = h/w trigger rising with exposure control
// 6 = h/w trigger falling with exposure control

LINKDLL		bool	PSL_VHR_set_required_operating_temp(int iOperatingTemp);
// set required operating temperature within range -55 to +45 degrees C
LINKDLL		int		PSL_VHR_read_current_temp(void);
// returns the current operating temperature of camera  in degrees C


LINKDLL bool PSL_VHR_Is_intensifier_fitted(void);
// checks whether camera has an intensifier fitted
LINKDLL bool PSL_VHR_WriteIntensifierGain(unsigned short intgain);
// sets intensifier gain in range 0 to 4095, returns false if intgain is out of range
// OR the camera is not fitted with an intensifier






//////////////////////////// acquisition functions ////////////////////////////////////////////////////
LINKDLL 	bool	PSL_VHR_Snap_and_return(void);
// starts a single 16 bit acquisition (use PSL_VHR_Get_snap_status() to check when finished)
LINKDLL		bool	PSL_VHR_abort_snap(void);
// aborts current acquisition, returns false if failed
LINKDLL 	bool	PSL_VHR_Get_snap_status(void);
// returns true when exposure is finished (or timed out), false if not. 
LINKDLL		bool	PSL_VHR_grab_exposed_image(void);
// grab the now exposed image and put in internal image buffer

LINKDLL 	unsigned short * PSL_VHR_Get_image_pointer(void);
// returns a pointer to the start of the image data. This functions should be called thus:
// PSL_VHR_Snap_and_return() -> PSL_VHR_Get_snap_status()==true -> PSL_VHR_grab_exposed_image() -> PSL_VHR_Get_image_pointer()

LINKDLL 	int		PSL_VHR_Return_width();
//reads current image width
LINKDLL 	int		PSL_VHR_Return_height();
//reads current image height

LINKDLL bool PSL_VHR_apply_post_snap_processing(unsigned short *pImage);
// applies any user enabled corrections (see above) to the image in pImage. This function should always
// be called BEFORE any calls to PSL_VHR_remap_image(), PSL_VHR_subtract_background_image() 
// or PSL_VHR_software_bin_image()




LINKDLL 	bool	PSL_VHR_Fusion_snap(unsigned short noise_reduction_factor);

LINKDLL 	unsigned long * PSL_VHR_Get_32bit_fusion_image_pointer(void); 
		
LINKDLL		void PSL_VHR_log_compress_32bit_fusion_to_16bit(unsigned long *orig_image,int image_width,int image_height);




	

// Please note: the functions below represent optional additional image processing steps and, if called, 
// must be called in the following sequence:
// 1. PSL_VHR_remap_image()
// 2. PSL_VHR_subtract_background_image()
// 3. PSL_VHR_software_bin_image()

//////////////////////////// remapping (distortion removal) functions /////////////////////////
LINKDLL unsigned short *
PSL_VHR_remap_image(unsigned short *orig_image,int *image_width,int *image_height,bool bSmoothingEnabled,bool bClippingEnabled);
// this remaps the just acquired image data in orig_image and returns a pointer to the remapped image. The dimensions 
// of the remapped image are returned in image_width and image_height

LINKDLL unsigned long *
PSL_VHR_remap_20bit_fusion_image(unsigned long *orig_image,int *image_width,int *image_height,bool bSmoothingEnabled,bool bClippingEnabled);





/////////////////////////// background image subtraction functions 
// (these functions are only available for normal 16 bit images)
LINKDLL bool PSL_VHR_load_background_file(void) ;
LINKDLL void PSL_VHR_set_background_subtraction_pedestal(int iBackPed);
LINKDLL bool PSL_VHR_subtract_background_image(unsigned short *source_image,int source_width, int source_height);




/////////////////////////// software binning functions ///////////////////////////////////////////////
LINKDLL void PSL_VHR_software_bin_image(unsigned short *orig_image,int *image_width,int *image_height,int xbinning,int ybinning);
// software bins image buffer pointed to by orig_image, whose dimensions are image_width*image_height pixels
// The binned image is left in orig_image, but the new dimensions are returned in image_width and image_height
LINKDLL void PSL_VHR_software_bin_20bit_fusion_image(unsigned long *orig_image,int *image_width,int *image_height,int xbinning,int ybinning);
// software bins a 20 bit fusion image








LINKDLL bool PSL_VHR_write_tiff16_file(unsigned short *orig_image,int image_width,int image_height,char *fname);
// writes an image buffer to file in 16 bit TIFF format




LINKDLL void	PSL_VHR_Free(void);
// release framegrabber and system resources







	















/////////// for internal use only- DO NOT USE /////////////////////////////////////////////////

LINKDLL bool	PSL_VHR_load_camera_configuration_file(void);

LINKDLL bool	PSL_VHR_open_structure_file(void);
LINKDLL void	PSL_VHR_enable_structure_correction(bool bStructCorrectEnabled);
	
LINKDLL bool	PSL_VHR_set_number_of_overscan_rows(int iOverscanRows);
LINKDLL void	PSL_VHR_enable_display_overscan(bool bDisp);
LINKDLL bool	PSL_VHR_enable_background_flushing(bool bEnable);

LINKDLL		int		 PSL_VHR_Return_number_of_operating_modes_supported(void);
LINKDLL 	bool	 PSL_VHR_Set_camera_operating_mode(long lOpmode);
 	// IS4022 has 2 operating modes:
	// 0 = 12Mhz		(faster clocking and lower noise) [default]
	// 1 = 12Mhz_x		(slower clocking)

LINKDLL		char * PSL_VHR_get_descriptive_text(void);
// get a string describing current acquisition settings as established
// via calls to various functions in this dll

LINKDLL 	bool	PSL_VHR_initialise_sequence_storage(unsigned short no_images);
LINKDLL 	bool	PSL_VHR_snap_sequence(unsigned short no_images_to_snap);
LINKDLL 	bool	PSL_VHR_get_sequence_image_pointer(unsigned short * *pSequenceImage,unsigned short image_required);
LINKDLL 	bool	PSL_VHR_free_sequence_storage(void);

//LINKDLL int PSL_VHR_read_temp_from_i2c(void); // I2C DEVELOPMENT

LINKDLL		int		 PSL_VHR_get_raw_image_size_in_pixels(void);

/////////// for internal use only /////////////////////////////////////////////////////////////











#ifdef __cplusplus
};
#endif



#endif /* _PSL_VHR_H_ */
