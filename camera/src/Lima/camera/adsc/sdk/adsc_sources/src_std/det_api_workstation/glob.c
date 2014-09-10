#include	"defs.h"

/*
 *	These module definitions are on the "receiving side"
 */
FILE			*fperr;
FILE			*fpout;
FILE			*fpdiag;
FILE			*fplog;
FILE			*fp_connect_log;

int			verbose = 1;

int 			n_ctrl;
int 			u_ctrl[MAX_CONTROLLERS];
BYTE			pv_bn[MAX_CONTROLLERS];
struct q_moddef		qmod[MAX_CONTROLLERS];
struct q_conkind	qc[MAX_CONTROLLERS];
time_t			tmp_sample_time;

char			somsg[1024];
char			semsg[1024];

double			read_temp_val[MAX_CONTROLLERS];
double			set_temp_value;
double			ramp_temp_value;

int 			diskbased;
int 			send_square;
int 			control_dets;
char			disk_dir[256];
int 			det_api_module;
int 			save_raw;
int 			transform_image;
int 			image_kind;
int 			data_fd[MAX_CONTROLLERS];
int				ccd_serial_number;
int				ccd_uniform_pedestal;
int				env_detector_sn;

struct chip_patch	mod_patches[MAX_CONTROLLERS][MAX_PATCHES];
int 			chip_npatches[MAX_CONTROLLERS];
struct chip_patch	mod_patches_hb[MAX_CONTROLLERS][MAX_PATCHES];
int 			chip_npatches_hb[MAX_CONTROLLERS];
int			ccd_temp_update_sec;
int			ccd_command_idle_sec;
int			ccd_cycle_temp_count;
int			ccd_send_bufsize;

/*
 *	mserver globals, threads, and mutexes.
 */

int			ccd_use_mserver;
int			mserver_table_length;
int			mserver_mem_limit;
int			mserver_mem_use;

struct mserver_buf	*dp_table;

int			mserver_active_index;
int			mserver_insert_index;

#ifdef unix

pthread_mutex_t	h_mutex_mem = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t	mserver_active_index_cond  = PTHREAD_COND_INITIALIZER;
pthread_mutex_t	mserver_insert_index_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t	h_mutex_table  = PTHREAD_COND_INITIALIZER;
pthread_t	mserver_dispatcher_thread;

#endif /* unix */

#ifdef WINNT

HANDLE		h_mserver_dispatcher_thread;
HANDLE		h_mutex_mem;
HANDLE		h_mutex_table;

#endif /* WINNT */

/*
 *	hw dispatcher mutexes, threads.
 */

int 	hw_cmd_no;
int 	hw_cmd_ret;
int 	hw_thread_running;

#ifdef unix

pthread_cond_t	h_mutex_hw_cmd  = PTHREAD_COND_INITIALIZER;
pthread_cond_t	h_mutex_hw_ret  = PTHREAD_COND_INITIALIZER;
pthread_cond_t	h_mutex_hw_th_start  = PTHREAD_COND_INITIALIZER;
pthread_t	h_hw_thread;
HANDLE		h_hw_cmd_start_event;
HANDLE		h_hw_cmd_finish_event;

#endif /* unix */

#ifdef WINNT

HANDLE		h_hw_thread;
HANDLE		h_mutex_hw_cmd;
HANDLE		h_mutex_hw_ret;
HANDLE		h_mutex_hw_th_start;
HANDLE		h_hw_cmd_start_event;
HANDLE		h_hw_cmd_finish_event;

#endif /* WINNT */

int 	command_s;              /* listens on this socket */
int 	data_s;                 /* listens on this socket */
int 	command_port_no;        /* TCPIP port number for commands */
int 	data_port_no;           /* TCPIP port number for data */
int 	command_fd;             /* file descriptor for commands after connection */
int 	sec_command_s;  /* for secondary command entry */
int 	sec_command_fd;
int 	sec_command_port_no;

char   *detterm = "end_of_det\n";

/*
 *      data base detector definition for the "sending side"
 */

struct q_moddef qm[MAX_CONTROLLERS];

/*
 *      host, port database
 */

int 	q_ncon;                                 /* number of connections which need to be made */
char	q_hostnames[MAX_CONTROLLERS][256];      /* host name */
int 	q_ports[MAX_CONTROLLERS];               /* port numbers */
int 	q_dports[MAX_CONTROLLERS];              /* data ports */
int 	q_sports[MAX_CONTROLLERS];              /* secondary command ports */


int 	q_blocks[MAX_CONTROLLERS][MAX_CONTROLLERS];     /* assigned data blocks from each connection */
int 	q_states[MAX_CONTROLLERS];
int 	q_issync[MAX_CONTROLLERS];

int 	q_nctrl;                                                                /* number of modules */

int	q_ncols_raw = -1;
int	q_nrows_raw = -1;
int	q_image_size = -1;

int 	ccd_detector_type = CCD_TYPE_T2K;

int 	ccd_adc;
int 	ccd_trigger_mode;
int 	ccd_row_bin;
double  ccd_exp_time;

int 	ccd_state = STATE_IDLE;
int 	ccd_hardware = CCD_HARDWARE_BAD;
int 	ccd_synch;			/* 1 for synchoronous operation, 0 for asynchronous operation */
int 	ccd_send_all_module_status;

char *reply_term = REPLY_TERM_STRING;

char    replybuf[RBUFSIZE];
int 	rbufind;

char    det_outbuf[20480];
char    det_reply_single[10240+512];
char    det_reply[20480+512];

char            inbuf[INBUFSIZE];
int                     inbufind;
int                     command_number;                                 /* from the command enum */
int                     processing_command;                             /* state variable during parse */
int                     input_header_size;                              /* current value for header size in bytes */
int                     input_header_ind;
char            input_header[CCD_HEADER_MAX];   /* storage for same */

char            ccd_info[256];                                  /* just an informational tag for log files */

struct input_pair command_list[] =
  {
        START_CMD,                      "start",
        STOP_CMD,                       "stop",
        FLUSH_CMD,                      "flush",
        ABORT_CMD,                      "abort",
        RESET_CMD,                      "reset",
        SETPARAM_CMD,           "setparam",
        GETPARAM_CMD,           "getparam",
        STATUS_CMD,                     "status",
        WAIT_CMD,                       "wait",
        EXIT_CMD,                       "exit",
        SHUTTER_CMD,            "shutter",
        TEMPREAD_CMD,           "temp_read",
        TEMPSET_CMD,            "temp_set",
        TEMPRAMP_CMD,           "temp_ramp",
        ABORTTEMP_CMD,          "abort_temp",
        STOPR_CMD,                      "stopr",
        STOPW_CMD,                      "stopw",
        FLUSHB_CMD,                     "flushb",
        FLUSHE_CMD,                     "flushe",
        HWRESET_CMD,            "hwreset",
        POWERUPINIT_CMD,        "powerupinit",
        LOADFILE_CMD,           "loadfile",
        TIMESYNC_CMD,           "timesync",
        -1,                                     NULL,
  };

struct input_pair modifier_list[] =
  {
        END_OF_DET_MOD,         "end_of_det",
        MODE_MOD,               "mode",
        TRIGGER_MOD,            "trigger",
        SYNCH_MOD,              "synch",
        TIME_MOD,               "time",
        TIMECHECK_MOD,          "timecheck",
        ADC_MOD,                "adc",
		HW_BIN_MOD,				"hw_bin",
        ROW_BIN_MOD,            "row_bin",
        COL_BIN_MOD,            "col_bin",
        ROW_OFF_MOD,            "row_off",
        COL_OFF_MOD,            "col_off",
        ROW_XFER_MOD,           "row_xfer",
        COL_XFER_MOD,           "col_xfer",
        HEADER_SIZE_MOD,        "header_size",
        INFO_MOD,               "info",
        ALL_MOD,                "all",
        PCSHUTTER_MOD,          "pcshutter",
        J5_TRIGGER_MOD,         "j5_trigger",
        STRIP_AVE_MOD,          "strip_ave",
        TEMP_READ_MOD,          "temp_read",
        TEMP_SET_MOD,           "temp_set",
        TEMP_RAMP_MOD,          "temp_ramp",
        TEMP_TARGET_MOD,        "temp_target",
        TEMP_FINAL_MOD,         "temp_final",
        TEMP_STATUS_MOD,        "temp_status",
        TEMP_INCREMENT_MOD,     "temp_increment",
        CONFIG_MOD,             "config",
        SAVE_RAW,               "save_raw",
        TRANSFORM_IMAGE,        "transform_image",
        IMAGE_KIND,             "image_kind",
        XFDATAFD_MOD,           "xfdatafd",
	STORED_DARK,		"stored_dark",
	LOADFILE,		"loadfile",
        -1,                     NULL
  };

int	use_stored_dark = 0;
int	use_loadfile = 0;
char	timecopy[256];
