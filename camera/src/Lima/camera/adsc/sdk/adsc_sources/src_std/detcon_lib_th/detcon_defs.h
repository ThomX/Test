#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	"../incl/ccdconv.h"
#include	"../incl/esd.h"
#include	"../incl/esd_com.h"
#include	"../incl/ccddefs.h"
#include	"../incl/ccdsys.h"
#include	<errno.h>
#include	<signal.h>

#include	<unistd.h>
#include	<sys/types.h>
#include	<sys/time.h>
#include	<sys/stat.h>
#include	<fcntl.h>
#include	<time.h>
#include	<sys/socket.h>
#include	<netdb.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<pthread.h>

#include	"../incl/detcon_par.h"
#include	"../incl/detcon_state.h"

#include	"../incl/filec.h"
#include	"detcon_fcns.h"

#define	GOOD_STATUS	0
#define	BAD_STATUS	1

#define	CCD_N_CTRL		"CCD_N_CTRL"

#define	CCD_LAST_IDLE		0
#define	CCD_LAST_START		1
#define CCD_LAST_STOP		2
#define CCD_LAST_GET		3

#define	CCD_DET_OK		0
#define	CCD_DET_RETRY		1
#define	CCD_DET_FATAL		2
#define	CCD_DET_NOTCONNECTED	3
#define	CCD_DET_DISCONNECTED	4


/*
 *	Some default values.
 */

#define SPECIFIC_PIXEL_SIZE	(0.051296)
#define	SPECIFIC_OUTFILE_TYPE	(0)
#define	SPECIFIC_DETECTOR_SN	(444)
#define	SPECIFIC_NO_TRANSFORM	(0)
#define	SPECIFIC_OUTPUT_RAWS	(0)
#define	SPECIFIC_J5_TRIGGER	(0)
#define SPECIFIC_CHIP_SIZE_X      (2048)
#define SPECIFIC_CHIP_SIZE_Y      (2048)
#define	SPECIFIC_T2K_DETECTOR	(1)
#define	SPECIFIC_MODULAR	(1)
#define	SPECIFIC_GATHER_RETURN	(0)
