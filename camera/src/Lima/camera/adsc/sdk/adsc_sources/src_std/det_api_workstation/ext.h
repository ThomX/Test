#include	"defs.h"

/*
 *	Miscellaneous stuff.
 */

extern	FILE	*fplog;				/* used for logging events to a file */
extern	FILE	*fperr;
extern	FILE	*fpout;
extern	FILE	*fpdiag;
extern	FILE	*fp_connect_log;

extern	int 	verbose;
/*
 *	Input buffering, etc.
 */

extern	char 	inbuf[INBUFSIZE];
extern	int 	inbufind;
extern	char 	replybuf[RBUFSIZE];
extern	int 	rbufind;
extern	int 	command_number;					/* from the command enum */
extern	int 	processing_command;				/* state variable during parse */
extern	int 	input_header_size;				/* current value for header size in bytes */
extern	int 	input_header_ind;
extern	char 	input_header[CCD_HEADER_MAX];	/* storage for same */
extern	char	ccd_info[256];					/* an informational tag for log files */
/*
 *      Simulation information.
 */
   
extern	int 	simulation_mode;                /* 1 if simulation, else 0 */
extern	int 	sim_dir[256];                   /* directory containing simulation images */
extern	int 	sim_fmt;                        /* format of simulation images */

extern	int 	n_ctrl;				/* number of controllers */
extern	int 	u_ctrl[MAX_CONTROLLERS];	/* mask of active controllers */
extern	struct q_moddef		qmod[MAX_CONTROLLERS];
extern	struct q_conkind	qc[MAX_CONTROLLERS];

extern	time_t  tmp_sample_time;

extern	char	somsg[1024];
extern	char	semsg[1024];

extern	double	read_temp_val[MAX_CONTROLLERS];
extern	double	set_temp_value;
extern	double	ramp_temp_value;
extern	BYTE	pv_bn[MAX_CONTROLLERS];

extern	int 	diskbased;
extern	int 	send_square;
extern	int 	control_dets;
extern	char	disk_dir[256];
extern	int 	det_api_module;
extern	int 	save_raw;
extern	int 	transform_image;
extern	int 	image_kind;
extern	int 	data_fd[MAX_CONTROLLERS];
extern	int	ccd_serial_number;
extern	int	ccd_uniform_pedestal;
extern	int		env_detector_sn;

extern	struct chip_patch       mod_patches[MAX_CONTROLLERS][MAX_PATCHES];
extern	int 		chip_npatches[MAX_CONTROLLERS];
extern	struct chip_patch       mod_patches_hb[MAX_CONTROLLERS][MAX_PATCHES];
extern	int 		chip_npatches_hb[MAX_CONTROLLERS];
extern	int		ccd_temp_update_sec;
extern	int		ccd_command_idle_sec;
extern	int		ccd_cycle_temp_count;
extern	int		ccd_send_bufsize;

extern	int		ccd_use_mserver;
extern	int		mserver_table_length;
extern	int		mserver_mem_limit;
extern	int		mserver_mem_use;
extern	struct mserver_buf        *dp_table;
extern	int 	mserver_active_index;
extern	int 	mserver_insert_index;

#ifdef unix

extern	pthread_mutex_t	h_mutex_mem;
extern	pthread_cond_t	mserver_active_index_cond;
extern	pthread_mutex_t	h_mutex_table;
extern	pthread_cond_t	mserver_insert_index_cond;
extern	pthread_t	mserver_dispatcher_thread;

#endif /* unix */

#ifdef WINNT

extern	HANDLE		h_mserver_dispatcher_thread;
extern	HANDLE		h_mutex_mem;
extern	HANDLE		h_mutex_table;
#endif /* WINNT */

extern	int 		hw_cmd_no;
extern	int 		hw_cmd_ret;
extern	int 		hw_thread_running;

#ifdef unix

extern	pthread_cond_t	h_mutex_hw_cmd;
extern	pthread_cond_t	h_mutex_hw_ret;
extern	pthread_cond_t	h_mutex_hw_th_start;
extern	pthread_t	h_hw_thread;
extern	HANDLE		h_hw_cmd_start_event;
extern	HANDLE		h_hw_cmd_finish_event;

#endif /* unix */

#ifdef WINNT

extern	HANDLE		h_hw_thread;
extern	HANDLE		h_mutex_hw_cmd;
extern	HANDLE		h_mutex_hw_ret;
extern	HANDLE		h_mutex_hw_th_start;
extern	HANDLE		h_hw_cmd_start_event;
extern	HANDLE		h_hw_cmd_finish_event;


#endif /* WINNT */

extern  int     command_port_no;
extern  int     data_port_no;

extern	int 	command_s;              /* listens on this socket */
extern	int 	command_s_sec;          /* listens on this socket */
extern	int 	data_s;                 /* listens on this socket */
extern	int 	command_port_no;        /* TCPIP port number for commands */
extern	int 	data_port_no;           /* TCPIP port number for data */
extern	int 	command_fd;             /* file descriptor for commands after connection */
extern	int 	sec_command_s;  /* for secondary command entry */
extern	int 	sec_command_fd;
extern	int 	sec_command_port_no;

extern  char    *detterm;

/*
 *      data base detector definition for the "sending side"
 */

extern	struct q_moddef qm[MAX_CONTROLLERS];

/*
 *      host, port database
 */

extern	int             q_ncon;                                 /* number of connections which need to be made */
extern	char    q_hostnames[MAX_CONTROLLERS][256];      /* host name */
extern	int             q_ports[MAX_CONTROLLERS];               /* port numbers */
extern	int             q_dports[MAX_CONTROLLERS];              /* data ports */
extern	int             q_sports[MAX_CONTROLLERS];              /* secondary command ports */


extern	int             q_blocks[MAX_CONTROLLERS][MAX_CONTROLLERS];     /* assigned data blocks from each connection */
extern	int             q_states[MAX_CONTROLLERS];
extern	int             q_issync[MAX_CONTROLLERS];

extern	int             q_nctrl;                                                                /* number of modules */

extern  int 	q_ncols_raw;
extern  int 	q_nrows_raw;
extern  int 	q_image_size;
extern	int 	ccd_detector_type;

extern  int             ccd_adc;
extern  int             ccd_trigger_mode;
extern  int             ccd_row_bin;
extern  double  ccd_exp_time;

extern	int 	ccd_state;
extern	int 	ccd_hardware;

extern	 char *reply_term;

extern	char	replybuf[RBUFSIZE];
extern	int 	rbufind;

extern  int 	ccd_synch;	/* 1 for synchoronous operation, 0 for asynchronous operation */
extern	int 	ccd_send_all_module_status;

extern	char    det_outbuf[20480];
extern	char    det_reply_single[10240 + 512];
extern	char    det_reply[20480 + 512];

extern	char            inbuf[INBUFSIZE];
extern	int                     inbufind;
extern	int                     command_number;                                 /* from the command enum */
extern	int                     processing_command;                             /* state variable during parse */
extern	int                     input_header_size;                              /* current value for header size in bytes */
extern	int                     input_header_ind;
extern	char            input_header[CCD_HEADER_MAX];   /* storage for same */

extern	char            ccd_info[256];                                  /* just an informational tag for log files */

extern	struct input_pair command_list[];

extern	struct input_pair modifier_list[];

extern	int	use_stored_dark;
extern	int	use_loadfile;
extern	char	timecopy[256];

extern	char	*char_ztime();
