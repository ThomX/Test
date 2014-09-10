#include	"detcon_ext.h"

/*
 *	These functions form the core of the detector control library.
 */

/*
 *	Begin an exposure.
 */

int	CCDStartExposure()
{
	dtc_last = CCD_LAST_START;
	exec_ccd_start_exposure();
	return(0);
}

/*
 *	Stop the current exposure, causing data to be transferred from
 *	the CCDs to local memory in the W95 PC.
 */

int	CCDStopExposure()
{
	dtc_last = CCD_LAST_STOP;
	exec_ccd_stop_exposure();
	return(0);
}

/*
 *	Return a string indicating the current state of the detector.
 *
 *	This string will be used to display status on the user GUI.
 */

char	*CCDStatus()
{
	return(dtc_status_string);
	return(0);
}

/*
 *	Return the state of the detector.  Enum in ../incl/detcon_state.h
 */

int	CCDState()
{
	return(dtc_state);
}

/*
 *	Set a file parameter.  Parameter enums found in ../incl/detcon_par.h
 */

int	CCDSetFilePar(int which_par, char *p_value)
{
	detcon_set_file_param(which_par,p_value);
	return(0);
}

/*
 *	Return a file parameter.  enums found in ../incl/detcon_par.h
 */

int	CCDGetFilePar(int which_par,char *p_value)
{
	detcon_get_file_param(which_par,p_value);
	return(0);
}

/*
 *	Set a Hardware Parameter.  enums found in ../incl/detcon_par.h
 */

int	CCDSetHwPar(int which_par,char *p_value)
{
	detcon_set_hw_param(which_par,p_value);
	return(0);
}

/*
 *	Return a Hardware Parameter.  Enums found in ../incl/detcon_par.h
 */

int	CCDGetHwPar(int which_par, char *p_value)
{
	detcon_get_hw_param(which_par,p_value);
	return(0);
}

/*
 *	Set binning.  1 for 1x1, 2 for 2x2.
 */

int	CCDSetBin(int val)
{
  int bin, adc;
  if(val == 1)
  {
    bin = 1;
    adc = 0;
  }
  else
  {
    bin = 2;
    adc = 1;
  }
  CCDSetHwPar(HWP_BIN, (char *) &bin);
  CCDSetHwPar(HWP_ADC, (char *) &adc);
  return(0);
}

/*
 *	Return current binning value.  1 for 1x1, 2 for 2x2.
 */

int	CCDGetBin()
{
  int temp;
	detcon_get_hw_param(HWP_BIN,(char *) & temp);
  return (temp);
}

/*
 *	Cause the image stored in the frame grabber, which has been previously
 *	transferred from the CCDs to local frame grabber memory, to be transmitted
 *	to the image_gather process.  This will cause whatever defaults are
 *	in effect (output raws, transform image, etc.) to be carried out.
 *	No further calls are required to produce appropriate files on disk.
 *
 *	"last" is 1 if this is the last image in a sequence, otherwise 0.
 *	This is required since the data is multi-buffered in the frame frabber PCs
 *	so it it necessary to know when this data must be "flushed".  This
 *	variable must be set to 1 for a single image or the last image in
 *	a run.
 */

int	CCDGetImage()
{
	dtc_last = CCD_LAST_GET;
	exec_ccd_get_image(dtc_lastimage);
	return(0);
}

/*
 *	This call is not necessary, but is here, since this funciton is
 *	automatically carried out by CCDGetImage above.
 */

int	CCDCorrectImage()
{
	return(0);
}

/*
 *	This call is not necessary, but is here, since this function is
 *	automatically carried out by CCDGetImage above.
 */

int	CCDWriteImage()
{
	exec_ccd_get_image(dtc_lastimage);
	return(0);
}

/*
 *	Return the last error, "none" if there is no error in effect.
 */

char	*CCDGetLastError()
{
	return(dtc_lasterror_string);
}

/*
 *	Abort the current image in progress, if any.
 */

int	CCDAbort()
{
	exec_ccd_abort();
	return(0);
}

/*
 *	Reset the state of the detector control software to a useful state.
 *	A call to this function will cause the detector to reset to a known
 *	state and various error indicators reset.  Designed to be called after
 *	an error occurs or an abort is issued.
 */

int	CCDReset()
{
	exec_reset();
	return(0);
}

/*
 *	Initialize the state of the CCD acquistion software.  This function MUST
 *	be called before ANY hardware functions may be called.  This call will
 *	bring up the network connections between the PC, the transform process,
 *	and this control library.  The "state" will be returned as "idle" when
 *	all connections are made and the control software is capable of collecting
 *	an image.
 */

int	CCDInitialize()
{
	exec_initialize();
	return(0);
}

int	CCD_HWReset()
{
	exec_hw_reset();
	return(0);
}

int	CCDSetProperty(char *property, const char *value, int overwrite)
{
	if(setenv(property, value, overwrite))
		return(1);
	else
		return(0);
}
