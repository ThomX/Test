#include	"detcon_defs.h"

extern	int	errno;		/* system */

extern char	dtc_lfname[2048];
extern char	dtc_confname[2048];

extern FILE	*dtc_fpout;
extern int	dtc_fdxfcm;
extern FILE	*dtc_fplog;
extern FILE	*dtc_fpconfig;
extern FILE	*dtc_connect_log;
extern int	dtc_fddetcmd;
extern int	dtc_fddetstatus;
extern int	dtc_fdblcmd;
extern int	dtc_fdblstatus;
extern int      dtc_xfdatafd;
extern int	dtc_fdsecdetcmd;
extern int	dtc_secdetport;

extern pthread_mutex_t	dtc_h_mutex_state;
extern pthread_t	dtc_exec_thread;
extern pthread_t	dtc_check_gather_return_thread;
extern pthread_t	dtc_temperature_thread;
extern pthread_t	dtc_server_thread;

extern	pthread_mutex_t         dtc_h_mutex_fddetcmd; 
extern	pthread_mutex_t         dtc_h_mutex_fdxfcm;

extern int	dtc_detector_sn;
extern int	dtc_no_transform;
extern int	dtc_output_raws;

extern char	dtc_default_suffix[20];
extern int	dtc_default_imsize;
extern int	dtc_state;
extern int	dtc_expos_msec_ticks;
extern int	dtc_initialized;
extern int	dtc_temp_enabled;
extern int	dtc_sec_initialized;
extern char	dtc_status_string[2048];
extern char	dtc_lasterror_string[2048];

extern FILE	*dtc_fpnull;
extern int	dtc_last;

extern	float	dtc_stat_dist;
extern	float	dtc_stat_lift;
extern	float	dtc_stat_phi;
extern	float	dtc_stat_omega;
extern	float	dtc_stat_kappa;
extern	int	dtc_wasset_kappa;
extern	float	dtc_stat_start_phi;
extern	float	dtc_stat_start_omega;
extern	float	dtc_stat_start_kappa;
extern	float	dtc_stat_osc_width;
extern	float	dtc_stat_time;
extern	float	dtc_stat_dose;
extern	float	dtc_stat_intensity;
extern	float	dtc_stat_wavelength;
extern	int	dtc_wasset_wavelength;
extern	int	dtc_stat_axis;
extern	int	dtc_stat_adc;
extern	int	dtc_stat_bin;
extern	float	dtc_stat_xcen;
extern	float	dtc_stat_ycen;
extern	float	dtc_stat_2theta;
extern	int	dtc_wasset_2theta;
extern	int	dtc_stat_compress;
extern	float	dtc_stat_dzratio;

extern	int	dtc_use_j5_trigger;

extern	int	dtc_stored_dark;
extern	int	dtc_force_dark;
extern	int	dtc_loadfile;
extern	char	dtc_read_filename[2048];
extern	char	dtc_userdef_str[2048];
extern	char	dtc_userret[20480];
extern	char    dtc_merge_header[20480];
extern	char	dtc_headerparams[20480];
extern	float	dtc_pixel_size;
extern	int	dtc_image_kind;
extern	int	dtc_lastimage;
extern	char	dtc_filename[1024];
extern	char	dtc_comment[2048];
extern	char	dtc_det_reply[102400];
extern	char	dtc_xform_reply[2048];
extern	int	dtc_temp_cold;
extern	int	dtc_temp_warm;
extern	int	dtc_temp_mode;
extern	float	dtc_temp_value;
extern	char	dtc_temp_status[10240];
extern	char	dtc_jpeg1_name[1024];
extern	char	dtc_jpeg2_name[1024];
extern	char	dtc_jpeg1_size[32];
extern	char	dtc_jpeg2_size[32];

extern	int	dtc_outfile_type;
extern	int	dtc_n_ctrl;
extern	int     dtc_chip_size_x;               /* chip size in x = number of columns */
extern	int     dtc_chip_size_y;               /* chip size in y = number of rows */
extern	int	dtc_t2k_detector;
extern	int	dtc_modular;
extern	int	dtc_check_gather_return;
extern	int     dtc_received_gather_error;
extern	int	dtc_autosample_temp_per_image;
extern	int	dtc_temps_transferred;
extern	int	dtc_mult_trigtype;
extern	int	dtc_mult_frame_start;
extern	int	dtc_mult_nframe;
extern	int	dtc_mult_settrigger;

extern	float	dtc_specific_pixel_size;
extern	int	dtc_specific_outfile_type;
extern	int	dtc_specific_detector_sn;
extern	int	dtc_specific_no_transform;
extern	int	dtc_specific_output_raws;
extern	int	dtc_specific_j5_trigger;
extern	int     dtc_specific_chip_size_x;               /* chip size in x = number of columns */
extern	int     dtc_specific_chip_size_y;               /* chip size in y = number of rows */
extern	int	dtc_specific_t2k_detector;
extern	int	dtc_specific_modular;
extern	int	dtc_specific_gather_return;
