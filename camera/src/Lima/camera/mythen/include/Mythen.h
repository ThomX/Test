#ifndef MYTHEN_H
#define MYTHEN_H

#define _LINUX

#ifdef __x86_64
	#define _x64
#else
	#define _x86
#endif

#include <mythenDetector.h>

#define GET_ACTION     mythenDetector::GET_ACTION
#define PUT_ACTION     mythenDetector::PUT_ACTION
#define READOUT_ACTION mythenDetector::READOUT_ACTION
#define MAX_ARGS       256
#define L_STRING       4096
#define M_STRING       1024
#define NSTRIP         (attr_NModules_write*1280)
#define MAXSTR         30720

#define S_DIM 4
const string settings_s[S_DIM] = {"standard", "fast", "highgain", "undefined"};

#define T_DIM 5
const string trimmode_s[T_DIM] = {"noise", "beam", "improve", "fix", "offline"};

#endif
