#ifndef DETCON_FCNS_H_INCLUDE
#define DETCON_FCNS_H_INCLUDE

/*
 *	These are the function definitions to the control library used for
 *	ADSC CCD detectors.  It is the standard ESRF TACO device server
 *	library.
 */

#ifdef __cplusplus
extern "C" {
#endif

int     CCDStartExposure();
int     CCDStopExposure();
char    *CCDStatus();
int     CCDState();
int     CCDSetFilePar(int which_par, char *p_value);
int     CCDGetFilePar(int which_par,char *p_value);
int     CCDSetHwPar(int which_par,char *p_value);
int     CCDGetHwPar(int which_par, char *p_value);
int     CCDSetBin(int val);
int     CCDGetBin();
int     CCDGetImage();
int     CCDCorrectImage();
int     CCDWriteImage();
char    *CCDGetLastError();
int     CCDAbort();
int     CCDReset();
int     CCDInitialize();
int     CCD_HWReset();
int	CCDSetProperty(char *property, const char *value, int overwrite);

#ifdef __cplusplus
}
#endif

#endif // DETCON_FCNS_H_INCLUDE
