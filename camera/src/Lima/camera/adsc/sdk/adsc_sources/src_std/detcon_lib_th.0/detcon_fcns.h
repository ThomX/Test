/*
 *	Thread-executed routines
 */

void	*detcon_send_det_start(void *arg);
void	*detcon_send_det_stop(void *arg);
void	*detcon_check_gather_return(void *arg);
void    *detcon_server_update(void *arg);
void	*detcon_set_temperature(void *arg);
void	*detcon_ramp_temperature(void *arg);

/*
 *	detcon_exec.c 
 */

int	exec_ccd_start_exposure();
int	exec_ccd_stop_exposure();
int	exec_ccd_get_image(int last);
int	exec_initialize();
int	exec_reset();
int	exec_hw_reset();
int	exec_ccd_abort();

/*
 *	detcon_tempcontrol.c
 */

int     temp_enable_temperature_control(int on);
int     temp_read_temperature();
int     temp_set_temperature(double val);
int     temp_ramp_temperature(double val);
int     temp_abort_temperature();

int	detcon_temp_par_cold(int getset);
int	detcon_temp_par_warm(int getset);
int	detcon_temp_par_mode(int getset);
int	detcon_temp_par_status(int getset);
int	detcon_temp_par_value(int getset);

/*
 *	Utility routines.
 */

void	detcon_sleep(int nmsec);
void	detcon_cleanexit(int status);
void    detcon_timestamp(FILE *fp);
void	detcon_log_state(int state, char *reason, int errnum);
char	*detcon_get_state_name(int state);
void	log_info(char *info, char * further, int errnum);
char	*dtc_ztime();

/*
 *	Thread, set state, and mutex related routines.
 */

int	detcon_start_detached_thread(void *handle, void*(fcn)(), void *arg, char *logging_tag);
void	setDTCState(int newState, char* reason, int errnum);

/*
 *	detcon_misc routines
 */

void    detcon_ccd_init_defaults();
int     detcon_ccd_initialize();
void    detcon_ccd_init_vars();
int     detcon_ccd_init_files();
int     detcon_ccd_init_config(FILE *fpmsg);

int	detcon_set_hw_param(int which_par, char *p_value);
int	detcon_set_file_param(int which_par, char *p_value);
int	detcon_get_hw_param(int which_par,char *p_value);
int	detcon_get_file_param(int which_par,char *p_value);
int	str_detcon_set_hw_param(int which_par, char *p_value);
int	str_detcon_set_file_param(int which_par, char *p_value);
int	str_detcon_get_hw_param(int which_par,char *p_value);
int	str_detcon_get_file_param(int which_par, char *p_value);

/*
 *	detcon_network.c
 */

int     detcon_connect_to_host_api(int *fdnet, char *host, int port, char *msg);
int	detcon_check_port_ready(int fd);
int     detcon_read_port_raw(int fd, char *stbuf, int stbufsize);
int     detcon_check_port_raw(int fd);
int     detcon_probe_port_raw(int fd);
int     detcon_probe_port_raw_with_timeout(int fd, int nmicrosecs);
int     detcon_check_for_connection(int fd);
int	detcon_notify_server_eof(int fd, int errno_received);

/*
 *	detcon_server.c
 */

void    detcon_make_header_smv();
int     detcon_read_temperature();
int     detcon_print_temp_readings(char *readings);
int     detcon_get_temperature_readings_fd(int fd);
int	detcon_temp_read_for_idle(int n_reads_left);
int     detcon_temp_read_for_stable(int n_reads_left);
int     detcon_temp_idle_when_finished(int val);
int     detcon_temp_stable_when_finished(int val);
int     detcon_abort_temperature();
int     detcon_get_stable_temperature_readings();
void    *detcon_send_det_stop(void *arg);
int     detcon_send_copy_command();
void    detcon_ccd_hw_initial_status();

int     detcon_output_detcmd(int fd, char *detcmd, char *hdptr, int hdsize);
int     detcon_output_detcmd_issue(int fd, char *detcmd, char *hdptr, int hdsize);
int     detcon_output_detcmd_receive(int fd);
int     detcon_output_gather_receive(int fd);
int     detcon_read_until(int fd, char *buf, int maxchar, char *looking_for);
int     detcon_rep_write(int fd, char *buf, int count);
int     detcon_server_init();
void    detcon_check_other_servers();
void    detcon_probe_other_servers();
int     detcon_server_enable_temperature_control(int on);
void	clock_heartbeat();
void	check_environ();
void	apply_reasonable_defaults();

/*
 *	detcon_entry.c
 */

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
