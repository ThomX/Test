#include	"detcon_defs.h"

char 	dtc_lfname[2048];		/* for logging important hardware messages */
char 	dtc_confname[2048];		/* configuration file name */

FILE	*dtc_fpout = NULL;		/* file pointer for output */
int	dtc_fdxfcm = -1;		/* file (socket) desc for image_gather command */
FILE	*dtc_fplog = NULL;		/* log file for useful info */
FILE	*dtc_fpconfig = NULL;		/* file pointer for config file */
FILE	*dtc_connect_log = NULL;	/* log file for network connections and disconnections */
int	dtc_fddetcmd = -1;		/* det_api_workstation command socket */
int     dtc_xfdatafd = -1;		/* 1 when we've seen a connected data socket from detector program */
int	dtc_fdsecdetcmd = -1;		/* file descriptor for secondary det_api_workstation command (temperature control) */
int	dtc_secdetport = -1;

pthread_mutex_t		dtc_h_mutex_state = PTHREAD_MUTEX_INITIALIZER;
pthread_t		dtc_exec_thread = (pthread_t) NULL;
pthread_t		dtc_check_gather_return_thread = (pthread_t) NULL;
pthread_t		dtc_temperature_thread = (pthread_t) NULL;
pthread_t		dtc_server_thread = (pthread_t) NULL;

pthread_mutex_t		dtc_h_mutex_fddetcmd = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t		dtc_h_mutex_fdxfcm = PTHREAD_MUTEX_INITIALIZER;

int	dtc_detector_sn;	/* serial number of the detector, if known */
int	dtc_output_raws;	/* 1 to output raws */
int	dtc_no_transform;	/* 1 to do no transform on line */

FILE	*dtc_fpnull;	/* /dev/null */
int	dtc_last = CCD_LAST_IDLE;

char	dtc_default_suffix[20] = ".img";
int	dtc_default_imsize = 4096 * 4096 * 2;
int	dtc_state = DTC_STATE_ERROR;
int	dtc_expos_msec_ticks;
int	dtc_initialized = 0;
int	dtc_sec_initialized = 0;
int	dtc_temp_enabled = 0;
char	dtc_status_string[2048];
char	dtc_lasterror_string[2048];

/*
 *	Scanner status information.
 */

float	dtc_stat_dist;		/* current distance */
float	dtc_stat_phi;		/* current phi */
float	dtc_stat_lift;		/* current lift */
float	dtc_stat_omega;		/* current omega */
float	dtc_stat_kappa;		/* current kappa */
int	dtc_wasset_kappa;	/* kappa was set */
float	dtc_stat_start_phi;		/* starting phi value for this scan */
float	dtc_stat_start_omega;	/* starting omega */
float	dtc_stat_start_kappa;	/* strting kappa */
float	dtc_stat_osc_width;		/* oscillation range */
float	dtc_stat_time;		/* exposure time */
float	dtc_stat_dose;		/* dose units, if applicable */
float	dtc_stat_intensity;		/* intensity reading from mar */
float	dtc_stat_wavelength;	/* wavelength */
int	dtc_wasset_wavelength;
int	dtc_stat_axis;		/* 1 for phi, 0 for omega */
int	dtc_stat_adc = 0;		/* adc value (slow=0, fast = 1) */
int	dtc_stat_bin = 1;		/* 1 for 1x1, 2 for 2x2 binning */
float	dtc_stat_xcen;		/* x beam center in mm for oblique correction */
float	dtc_stat_ycen;		/* y beam center in mm for oblique correction */
float	dtc_stat_2theta;		/* two theta of the detector */
int	dtc_wasset_2theta;
int	dtc_stat_compress;		/* 0 for none, 1 for .Z 2 for .pck */
int	dtc_stat_anom;		/* 1 for anomalous, else 0 */
int	dtc_stat_wedge;		/* number of frames per anom mini-run */
float	dtc_stat_dzratio = 1.0;	/* ratio of 2nd/1st image expos time/dezingering */
int	dtc_chip_size_x;		/* chip size in x = number of columns */
int	dtc_chip_size_y;		/* chip size in y = number of rows */
int	dtc_t2k_detector;		/* 1 if Q210/315 detector */
int	dtc_modular;			/* 1 if using modular framegrabbers */
int	dtc_check_gather_return;		/* 1 to have image write collect image gather replies */
int	dtc_received_gather_error;	/* If get a ccd_image_gather error, set to 1 */
int	dtc_autosample_temp_per_image;	/* 1 to automatically update temperature after each image */
int	dtc_temps_transferred;		/* 1 if we have transferred temperatures one or more times */
int	dtc_mult_trigtype = -1;		/* multi-image trigger type, -1 = not defined (not a multi-frame sequence), 0 = sw trigger, 1 hw trigger */
int	dtc_mult_frame_start = 0;	/* multi-image first frame number >= 0 */
int	dtc_mult_nframe = 0;		/* multi-image set number of frames to collect >= 1 */
int	dtc_mult_settrigger = 0;	/* multi-image, 1 = sw trigger readout sequence; initial value 0; reset to zero after set to 1 */


/*
 *	Simulation timing and control.
 */

int	dtc_use_j5_trigger;

/*
 *	Variables which apply to both simulated and actual
 *	hardware operation.
 */

int	dtc_stored_dark;
int	dtc_force_dark;
int	dtc_loadfile = 0;
char	dtc_read_filename[2048];
char	dtc_userdef_str[2048];
char	dtc_userret[20480];
char	dtc_merge_header[20480];
char	dtc_headerparams[20480];
int	dtc_pixel_size;
int	dtc_image_kind;
int	dtc_lastimage;
char	dtc_filename[1024];
char	dtc_comment[2048];
char	dtc_det_reply[102400];
char	dtc_xform_reply[2048];
int	dtc_temp_cold = 0;
int	dtc_temp_warm = 0;
int	dtc_temp_mode = 0;
int	dtc_temp_value = 0;
char	dtc_temp_status[10240];
char	dtc_jpeg1_name[1024];
char	dtc_jpeg2_name[1024];
char	dtc_jpeg1_size[32];
char	dtc_jpeg2_size[32];

int	dtc_n_ctrl;				/* number of controllers */

/*
 *	Some timing variables useful to have.
 */

int	dtc_outfile_type;		/* 0 or ushort, 1 for int */

/*
 *	These are lower case assigned versions of
 *	the default #defines in the mdcdefs.h file.
 *
 *	They are given the initial values by the .h file
 *	but may be overridden either by the program
 * 	or by the user's configuration file.
 */


float	dtc_specific_pixel_size;		/* pixel size */
int	dtc_specific_compress;		/* 0 for none, 1 for .Z 2 for .pck */
int	dtc_specific_dk_before_run;		/* 1 for dark current repeat before a run */
int	dtc_specific_repeat_dark;		/* 1 for repeat dark current */
int	dtc_specific_outfile_type;		/* 0 or ushort, 1 for int */
int	dtc_specific_detector_sn;		/* detector serial number, if specified */
int	dtc_specific_no_transform;		/* 1 for no on-line transform */
int	dtc_specific_output_raws;		/* 1 for output raws */
int	dtc_specific_j5_trigger;		/* 1 to use j5 trigger for ext_sync */
int	dtc_specific_chip_size_x;		/* chip size in x = number of columns */
int	dtc_specific_chip_size_y;		/* chip size in y = number of rows */
int	dtc_specific_t2k_detector;
int	dtc_specific_modular;
int	dtc_specific_gather_return;	/* 1 to have image write wait for xform reply */
