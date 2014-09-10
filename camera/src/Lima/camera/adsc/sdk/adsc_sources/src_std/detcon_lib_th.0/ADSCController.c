#include <jni.h>
#include "gda_px_detector_ADSCController.h"
#include <stdio.h>
#include "detcon_entry.h"
#include "detcon_par.h"
#include <math.h>
//#include <time.h>

/* Use:
 * 
 *    const char *str = (*env)->GetStringUTFChars(env, p, 0);
 *    .......
 *    (*env)->ReleaseStringUTFChars(env, p, str);
 *
 * to convert from and back to java strings and release memory.
 */ 

static int returnCode;
static int detStat;

static  int manual_darks;
static  int transform;
static  int flp_kind;

static jfieldID rcFID = NULL;
static jfieldID dsFID = NULL;
static jfieldID statFID = NULL;
static jfieldID lerrFID = NULL;
static jfieldID fpFID = NULL;
static jfieldID hpFID = NULL;

JNIEXPORT void JNICALL 
Java_gda_px_detector_ADSCController_initFIDs (JNIEnv *env, jclass cls)
{
  rcFID = (*env)->GetFieldID(env, cls, "returnCode", "I");
  dsFID = (*env)->GetFieldID(env, cls, "detectorStatus", "I");
  statFID = (*env)->GetFieldID(env, cls, "statusString", "Ljava/lang/String;");
  lerrFID = (*env)->GetFieldID(env, cls, "errorString", "Ljava/lang/String;");
  fpFID = (*env)->GetFieldID(env, cls, "filePar", "Ljava/lang/String;");
  hpFID = (*env)->GetFieldID(env, cls, "hardwarePar", "Ljava/lang/String;");
} 

void setFields (JNIEnv *env, jobject obj)
{
  (*env)->SetIntField(env, obj, rcFID, returnCode);
  (*env)->SetIntField(env, obj, dsFID, detStat);
}

JNIEXPORT jint JNICALL 
Java_gda_px_detector_ADSCController_doCCDStartExposure (JNIEnv *env, jobject obj)
{
//    clock_t start_expose_time = clock();
    CCDStartExposure ();
//    clock_t end_expose_time = clock();
//	 int clock_cycles = end_expose_time - start_expose_time;
//	 fprintf (stderr,  "!!!!!!!!!!!!!!!! cycles to run doCCDStartExposure: %d\n", clock_cycles);
    return(0);
}

JNIEXPORT jint JNICALL 
Java_gda_px_detector_ADSCController_doCCDStopExposure (JNIEnv *env, jobject obj)
{
    CCDStopExposure ();
    return(0);
}

JNIEXPORT jint JNICALL 
Java_gda_px_detector_ADSCController_doCCDStatus (JNIEnv *env, jobject obj)
{
  char *status;
  jstring jstr;

  status = CCDStatus();
  jstr = (*env)->NewStringUTF(env, status);
  if (jstr == NULL)
    return(0);
  (*env)->SetObjectField(env, obj, statFID, jstr);

  return(0);
}

JNIEXPORT jint JNICALL 
Java_gda_px_detector_ADSCController_doCCDState (JNIEnv *env, jobject obj)
{
//     clock_t start_expose_time = clock();
   detStat =  CCDState();
    setFields(env, obj);
//     clock_t end_expose_time = clock();
//	 int clock_cycles = end_expose_time - start_expose_time;
//	 fprintf (stderr,  "!!!!!!!!!!!!!!!! cycles to update fields in doCCDState: %d\n", clock_cycles);
   return(0);
}

JNIEXPORT jint JNICALL 
Java_gda_px_detector_ADSCController_doCCDSetFileName (JNIEnv *env, jobject obj, 
					    jstring p)
{
    const char *str = (*env)->GetStringUTFChars(env, p, 0);
    CCDSetFilePar(FLP_FILENAME, str);
    /*CCDSetFilePar(FLP_KIND, &flp_kind);*/
    (*env)->ReleaseStringUTFChars(env, p, str);
    return(0);
}

JNIEXPORT jint JNICALL 
Java_gda_px_detector_ADSCController_doCCDSetReadFileName (JNIEnv *env, jobject obj, 
					    jstring p)
{
    const char *str = (*env)->GetStringUTFChars(env, p, 0);
    CCDSetFilePar(FLP_READ_FILENAME, str);
    /*CCDSetFilePar(FLP_KIND, &flp_kind);*/
    (*env)->ReleaseStringUTFChars(env, p, str);
    return(0);
}

JNIEXPORT jint JNICALL 
Java_gda_px_detector_ADSCController_doCCDSetHeaderTime
(JNIEnv *env, jobject obj, jdouble time)
{
   float ti = time;
   
   CCDSetFilePar (FLP_TIME, (void *) &ti);
   return(0);
}

JNIEXPORT jint JNICALL 
Java_gda_px_detector_ADSCController_doCCDSetHeaderOscillation
(JNIEnv *env, jobject obj, jdouble oscillation)
{
   float osc = oscillation;
   
   CCDSetFilePar (FLP_OSC_RANGE,(void *) &osc);
   return(0);
}

JNIEXPORT jint JNICALL 
Java_gda_px_detector_ADSCController_doCCDSetHeaderPhi
(JNIEnv *env, jobject obj, jdouble phiAngle)
{
   float phi = phiAngle;
   int phiAxis = 1;         /* Phi is 1 Omega is 0 */
   
   CCDSetFilePar (FLP_AXIS, (void *) &phiAxis);
   CCDSetFilePar (FLP_PHI, (void *) &phi);
   return(0);
}

JNIEXPORT jint JNICALL 
Java_gda_px_detector_ADSCController_doCCDSetHeaderDistance
(JNIEnv *env, jobject obj, jdouble distance)
{
   float dist = distance;
   
   CCDSetFilePar (FLP_DISTANCE, (void *) &dist);
   return(0);
}

JNIEXPORT jint JNICALL 
Java_gda_px_detector_ADSCController_doCCDSetHeaderWavelength
(JNIEnv *env, jobject obj, jdouble wavelength)
{
   float wave = wavelength;
   
   CCDSetFilePar (FLP_WAVELENGTH, (void *) &wave);
   return(0);
}

JNIEXPORT jint JNICALL 
Java_gda_px_detector_ADSCController_doCCDSetHeaderBeamX
(JNIEnv *env, jobject obj, jdouble beamX)
{
   float x = beamX ;
   
   CCDSetFilePar (FLP_BEAM_X, (void *) &x);
   return(0);
}

JNIEXPORT jint JNICALL 
Java_gda_px_detector_ADSCController_doCCDSetHeaderBeamY
(JNIEnv *env, jobject obj, jdouble beamY)
{
   float y = beamY;
   
   CCDSetFilePar (FLP_BEAM_Y, (void *) &y);
   return(0);
}

JNIEXPORT jint JNICALL
Java_gda_px_detector_ADSCController_doCCDGetFilePar (JNIEnv *env, jobject obj, 
					    jint par)
{
  char filePar[120];
  jstring jstr;

  CCDGetFilePar (par, filePar);

  jstr = (*env)->NewStringUTF(env, filePar);
  if (jstr == NULL)
    return(0);

  (*env)->SetObjectField(env, obj, fpFID, jstr);
  
  return(0);
}

JNIEXPORT jint JNICALL 
Java_gda_px_detector_ADSCController_doCCDGetHwPar (JNIEnv *env, jobject obj, 
					  jint par)
{
  char hwPar[120];
  jstring jstr;

  CCDGetHwPar (par, hwPar);

  jstr = (*env)->NewStringUTF(env, hwPar);
  if (jstr == NULL)
    return(0);

  (*env)->SetObjectField(env, obj, hpFID, jstr);
 
  return(0);
}

JNIEXPORT jint JNICALL 
Java_gda_px_detector_ADSCController_doCCDSetReadFile (JNIEnv *env, jobject obj)
{
    int readFile = 1;
    CCDSetHwPar(HWP_LOADFILE, (void * ) &readFile);
    return(0);
}

JNIEXPORT jint JNICALL 
Java_gda_px_detector_ADSCController_doCCDClearReadFile (JNIEnv *env, jobject obj)
{
    int readFile = 0;
    CCDSetHwPar(HWP_LOADFILE, (void * ) &readFile);
    return(0);
}

// switch to software binning
JNIEXPORT jint JNICALL
Java_gda_px_detector_ADSCController_doCCDSoftBin (JNIEnv *env, jobject obj)
{
    int bin = 2;
    int adc = 0;
    CCDSetHwPar(HWP_BIN, (void * ) &bin);
    CCDSetHwPar(HWP_ADC, (void * ) &adc);
    return(0);
}


// switch to hardware binning
JNIEXPORT jint JNICALL
Java_gda_px_detector_ADSCController_doCCDHardBin (JNIEnv *env, jobject obj)
{
    int bin = 2;
    int adc = 1;
    CCDSetHwPar(HWP_BIN, (void * ) &bin);
    CCDSetHwPar(HWP_ADC, (void * ) &adc);
    return(0);
}

//no binning
JNIEXPORT jint JNICALL
Java_gda_px_detector_ADSCController_doCCDNoBin (JNIEnv *env, jobject obj)
{
    int bin = 1;
    int adc = 0;
    CCDSetHwPar(HWP_BIN, (void * ) &bin);
    CCDSetHwPar(HWP_ADC, (void * ) &adc);
    return(0);
}

//control if using stored darks
JNIEXPORT jint JNICALL
Java_gda_px_detector_ADSCController_doCCDUseStoredDark (JNIEnv *env, jobject obj)
{
    int use_dark = 1;
    CCDSetHwPar(HWP_STORED_DARK, (void * ) &use_dark);
    return(0);
}

JNIEXPORT jint JNICALL
Java_gda_px_detector_ADSCController_doCCDNoUseStoredDark (JNIEnv *env, jobject obj)
{
    int use_dark = 0;
    CCDSetHwPar(HWP_STORED_DARK, (void * ) &use_dark);
    return(0);
}

/*JNIEXPORT jint JNICALL 
Java_gda_px_detector_ADSCController_doCCDSetBin (JNIEnv *env, jobject obj)
{
    int bin = 2;
    CCDSetHwPar(HWP_BIN, (void * ) &bin);
    return(0);
}
JNIEXPORT jint JNICALL 
Java_gda_px_detector_ADSCController_doCCDClearBin (JNIEnv *env, jobject obj)
{
    int bin = 1;
    CCDSetHwPar(HWP_BIN, (void * ) &bin);
    return(0);
}

JNIEXPORT jint JNICALL 
Java_gda_px_detector_ADSCController_doCCDGetBin (JNIEnv *env, jobject obj)
{
    CCDGetBin ();
    return(0);
}*/

JNIEXPORT jint JNICALL
Java_gda_px_detector_ADSCController_doCCDSlowAdc  (JNIEnv *env, jobject obj)
{
    int adc = 0;

    CCDSetHwPar (HWP_ADC, (void * ) &adc);
    return(0);
}

JNIEXPORT jint JNICALL
Java_gda_px_detector_ADSCController_doCCDFastAdc  (JNIEnv *env, jobject obj)
{
    int adc = 1;

    CCDSetHwPar (HWP_ADC, (void * ) &adc);
    return(0);
}

JNIEXPORT jint JNICALL
Java_gda_px_detector_ADSCController_doCCDSaveRaw (JNIEnv *env, jobject obj)
{
    int raw = 1;

    CCDSetHwPar (HWP_SAVE_RAW, (void * ) &raw);
    return(0);
}

JNIEXPORT jint JNICALL
Java_gda_px_detector_ADSCController_doCCDNoRaw (JNIEnv *env, jobject obj)
{
    int raw = 0;

    CCDSetHwPar (HWP_SAVE_RAW, (void * ) &raw);
    return(0);
}

JNIEXPORT jint JNICALL 
Java_gda_px_detector_ADSCController_doCCDSetXform (JNIEnv *env, jobject obj)
{
    int xform = 0;
    CCDSetHwPar(HWP_NO_XFORM, (void * ) &xform);
    return(0);
}

JNIEXPORT jint JNICALL 
Java_gda_px_detector_ADSCController_doCCDClearXform (JNIEnv *env, jobject obj)
{
    int xform = 1;
    CCDSetHwPar(HWP_NO_XFORM, (void * ) &xform);
    return(0);
}

JNIEXPORT jint JNICALL 
Java_gda_px_detector_ADSCController_doCCDSetDark0 (JNIEnv *env, jobject obj)
{
    int dark_type = 0;
    flp_kind = dark_type;
    CCDSetFilePar(FLP_KIND, (void *) &dark_type);
    return(0);
}

JNIEXPORT jint JNICALL 
Java_gda_px_detector_ADSCController_doCCDSetDark1 (JNIEnv *env, jobject obj)
{
    int dark_type = 1;
    flp_kind = dark_type;
    CCDSetFilePar(FLP_KIND, (void *) &dark_type);
    return(0);
}

JNIEXPORT jint JNICALL 
Java_gda_px_detector_ADSCController_doCCDClearDark (JNIEnv *env, jobject obj)
{
    flp_kind = 5;
    CCDSetFilePar(FLP_KIND, (void *) &flp_kind);
    return(0);
}

JNIEXPORT jint JNICALL 
Java_gda_px_detector_ADSCController_doCCDAutoDarkMode (JNIEnv *env, jobject obj)
{
    int mode = 1;
    CCDSetHwPar (HWP_LOADFILE, (void * ) &mode);
    return(0);
}

JNIEXPORT jint JNICALL 
Java_gda_px_detector_ADSCController_doCCManualDarkMode (JNIEnv *env, jobject obj)
{
    int mode = 0;
    CCDSetHwPar (HWP_LOADFILE, (void * ) &mode);
    return(0);
}

JNIEXPORT jint JNICALL 
Java_gda_px_detector_ADSCController_doCCDSetLastImage (JNIEnv *env, jobject obj)
{
    int last = 1;

    CCDSetFilePar (FLP_LASTIMAGE, (void *) &last);
    return(0);
}

JNIEXPORT jint JNICALL 
Java_gda_px_detector_ADSCController_doCCDClearLastImage (JNIEnv *env, jobject obj)
{
    int last  = 0;

    CCDSetFilePar (FLP_LASTIMAGE, (void *) &last);
    return(0);
}

JNIEXPORT jint JNICALL 
Java_gda_px_detector_ADSCController_doCCDGetImage (JNIEnv *env, jobject obj)
{
    CCDGetImage ();
    return(0);
}

JNIEXPORT jint JNICALL 
Java_gda_px_detector_ADSCController_doCCDCorrectImage (JNIEnv *env, jobject obj)
{
    CCDCorrectImage ();
    return(0);
}

JNIEXPORT jint JNICALL 
Java_gda_px_detector_ADSCController_doCCDWriteImage (JNIEnv *env, jobject obj)

{
    CCDWriteImage ();
    return(0);
}

JNIEXPORT jint JNICALL 
Java_gda_px_detector_ADSCController_doCCDGetLastError (JNIEnv *env, jobject obj)
{
  char *buf;
  jstring jstr;

  buf = CCDGetLastError ();
  jstr = (*env)->NewStringUTF(env, buf);
  if (jstr == NULL)
    return(0);
  (*env)->SetObjectField(env, obj, lerrFID, jstr);

  return(0);

}

JNIEXPORT jint JNICALL 
Java_gda_px_detector_ADSCController_doCCDAbort (JNIEnv *env, jobject obj)
{
    CCDAbort ();
    return(0);
}

JNIEXPORT jint JNICALL
Java_gda_px_detector_ADSCController_doCCDReset (JNIEnv *env, jobject obj)
{
    CCDReset ();
    return(0);
}



JNIEXPORT jint JNICALL 
Java_gda_px_detector_ADSCController_doCCDInitialize (JNIEnv *env, jobject obj) 
{
  int xform = 0;
  int mode = 1;
  int adc = 0;

  CCDInitialize();
  system("sleep 5");

  CCDSetHwPar(HWP_ADC, (void * ) &adc);
  CCDSetHwPar(HWP_NO_XFORM, (void * ) &xform);

  return(0);
}
