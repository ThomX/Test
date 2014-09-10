#include	"detcon_ext.h"

/*
 *	Module to handle the network connections for
 *	the data collection server.
 *
 *	First, detcon_server_init() is called to initialize
 *	the network handling code.
 *
 *	Second, detcon_server_update() is called on its own thread
 *	to determine if a connection is requested.  If so,
 *	a notation is made for this socket and an accept is done.
 *
 */

static struct serverlist        xfserver;
static struct serverlist	dtserver;

void	detcon_catch_sigpipe()
{
	fprintf(stderr,"detcon_server: caught SIGPIPE signal\n");
}

void	detcon_timestamp(FILE *fp)
{
	long	clock;

	time(&clock);
	fprintf(fp,"timestamp: %s",(char *) ctime(&clock));
	fflush(fp);
}

int	detcon_server_init()
{
	char	*cp;

	signal(SIGPIPE,detcon_catch_sigpipe);

	if(NULL != (cp = getenv("CCD_DTSECPORT")))
	{
		dtc_secdetport = atoi(cp);
	}
	else
		dtc_secdetport = -1;

	if(NULL == dtc_connect_log)
	{
		if(NULL != (cp = getenv("CCD_DETCON_CONNECTION_LOG")))
		{
			if(NULL == (dtc_connect_log = fopen(cp, "a")))
			{
				fprintf(stderr, "WARNING:>>>\n\nConnection history file %s could not be created or opened.\n", cp);
				fprintf(stderr, "Detector control API will allow operation, but this problem should be corrected.\n");
				fprintf(stderr, "\n<<<WARNING\n\n");
			}
			else
			{
				fprintf(dtc_connect_log, "\n=========================================================================\n");
				fprintf(dtc_connect_log, "\n\tlibdetcon_mp started at %s\n\n", dtc_ztime());
				fprintf(dtc_connect_log, "\n=========================================================================\n");
			}
		}
	}

	if(NULL == (dtserver.sl_hrname = getenv("CCD_DTHOSTNAME")))
	{
		log_info("detcon_server_init", "ERROR: Environ CCD_DTHOSTNAME not defined", 0);
		return(1);
	}
	if(NULL == (cp = getenv("CCD_DTPORT")))
	{
		log_info("detcon_server_init", "ERROR: Environ CCD_DTPORT not defined", 0);
		return(1);
	}
	else
		dtserver.sl_port = atoi(cp);
	if(NULL == (xfserver.sl_hrname = getenv("CCD_XFHOSTNAME")))
	{
		log_info("detcon_server_init", "ERROR: Environ CCD_XFHOSTNAME not defined", 0);
		return(1);
	}
	if(NULL == (cp = getenv("CCD_XFPORT")))
	{
		log_info("detcon_server_init", "ERROR: Environ CCD_XFPORT not defined", 0);
		return(1);
	}
	else
		xfserver.sl_port = atoi(cp);
	return(0);
}

/*
 *	Probe servers (det_api_workstation, ccd_image_gather) which are supposed to be connected
 *	to see if they still are connected.  If disconnected, call notify_server_eof.  Call
 *	check_other_servers to reconnect anything which might be pending.
 */

void	detcon_probe_other_servers()
{
	if(0 == pthread_mutex_trylock(&dtc_h_mutex_fddetcmd))
	{
		if(-1 != dtc_fddetcmd)
			if(-1 == detcon_probe_port_raw(dtc_fddetcmd))
				detcon_notify_server_eof(dtc_fddetcmd, 0);
		pthread_mutex_unlock(&dtc_h_mutex_fddetcmd);
	}

	if(0 == pthread_mutex_trylock(&dtc_h_mutex_fdxfcm))
	{
		if(-1 != dtc_fdxfcm)
			if(-1 == detcon_probe_port_raw(dtc_fdxfcm))
				detcon_notify_server_eof(dtc_fdxfcm, 0);
		pthread_mutex_unlock(&dtc_h_mutex_fdxfcm);
	}

	detcon_check_other_servers();
}

/*
 *	Check for pending connections from other servers (det_api_workstation, ccd_image_gather).
 */

void	detcon_check_other_servers()
{
	char	buf[256];

	if(-1 == dtc_fddetcmd)
	{
	    if(-1 != detcon_connect_to_host_api(&dtc_fddetcmd,dtserver.sl_hrname,dtserver.sl_port,NULL))
	    {
		sprintf(buf, "connection OPEN with %s (fd: %d) for detector control", dtserver.sl_hrname, dtc_fddetcmd);
		log_info("detcon_connect_to_host_api", buf, 0);
		detcon_timestamp(dtc_fplog);
		fprintf(dtc_fplog,"detcon_server: connection established with hostname: %s with fd: %d for detector control\n",
			dtserver.sl_hrname,dtc_fddetcmd);
		fflush(dtc_fplog);
		if(NULL != dtc_connect_log)
		{
			fprintf(dtc_connect_log, 
				"detcon_server: at %s: connection OPENED with hostname: %s with fd: %d for detector control.\n\n", 
					dtc_ztime(), dtserver.sl_hrname,dtc_fddetcmd);
			fflush(dtc_connect_log);
		}
	    }
	}
	if(-1 == dtc_fdxfcm)
	{
	    if(-1 != detcon_connect_to_host_api(&dtc_fdxfcm,xfserver.sl_hrname,xfserver.sl_port,NULL))
	    {
		sprintf(buf, "connection OPEN with %s (fd: %d) for image_gather control", xfserver.sl_hrname, dtc_fdxfcm);
		log_info("detcon_connect_to_host_api", buf, 0);
		detcon_timestamp(dtc_fplog);
		fprintf(dtc_fplog,"detcon_server: connection established with hostname: %s with fd: %d for image_gather control\n",
			xfserver.sl_hrname,dtc_fdxfcm);
		fflush(dtc_fplog);
		if(NULL != dtc_connect_log)
		{
			fprintf(dtc_connect_log, 
				"detcon_server: at %s: connection OPENED with hostname: %s with fd: %d for image_gather control.\n\n", 
					dtc_ztime(), xfserver.sl_hrname,dtc_fdxfcm);
			fflush(dtc_connect_log);
		}
	    }
	}
}

int	detcon_notify_server_eof(int fd, int errno_received)
{
	char	buf[256];

	int	save_errno;

	save_errno = errno_received;
	if(fd == dtc_fdxfcm)
	{
		sprintf(buf, "connection CLOSED with %s (fd: %d) for image_gather control", dtserver.sl_hrname, dtc_fdxfcm);
		log_info("detcon_notify_server_eof", buf, errno);
		fprintf(dtc_fplog, 
			"detcon_server: at %s: connection CLOSED with hostname: %s with fd: %d for image_gather control.\n", 
				dtc_ztime(), xfserver.sl_hrname,dtc_fdxfcm);
		fprintf(dtc_fplog, "detcon_server: Errno was: %d (may or may not be relevant).\n\n", save_errno);
		fflush(dtc_fplog);

		if(NULL != dtc_connect_log)
		{
			fprintf(dtc_connect_log, 
				"detcon_server: at %s: connection CLOSED with hostname: %s with fd: %d for image_gather control.\n", 
					dtc_ztime(), xfserver.sl_hrname,dtc_fdxfcm);
			fprintf(dtc_connect_log, "detcon_server: Errno was: %d (may or may not be relevant).\n\n", save_errno);
			fflush(dtc_connect_log);
		}
		close(dtc_fdxfcm);
		dtc_fdxfcm = -1;
		return(0);
	}
	else if(fd == dtc_fddetcmd)
	{
		sprintf(buf, "connection CLOSED with %s (fd: %d) for detector control", dtserver.sl_hrname, dtc_fddetcmd);
		log_info("detcon_notify_server_eof", buf, errno);
		fprintf(dtc_fplog, 
			"detcon_server: at %s: connection CLOSED with hostname: %s with fd: %d for detector control.\n", 
				dtc_ztime(), dtserver.sl_hrname,dtc_fddetcmd);
		fprintf(dtc_fplog, "detcon_server: Errno was: %d (may or may not be relevant).\n\n", save_errno);
		fflush(dtc_fplog);

		if(NULL != dtc_connect_log)
		{
			fprintf(dtc_connect_log, 
				"detcon_server: at %s: connection CLOSED with hostname: %s with fd: %d for detector control.\n", 
					dtc_ztime(), dtserver.sl_hrname,dtc_fddetcmd);
			fprintf(dtc_connect_log, "detcon_server: Errno was: %d (may or may not be relevant).\n\n", save_errno);
			fflush(dtc_connect_log);
		}
		close(dtc_fddetcmd);
		dtc_fddetcmd = -1;
		return(0);
	}
	else if(fd == dtc_fdsecdetcmd)
	{
		sprintf(buf, "connection CLOSED with %s (fd: %d) for detector temperature control", dtserver.sl_hrname, dtc_fdsecdetcmd);
		log_info("detcon_notify_server_eof", buf, errno);
		fprintf(stderr, 
			"detcon_server: at %s: connection CLOSED with hostname: %s with fd: %d for secondary detector control (temperature).\n", 
				dtc_ztime(), dtserver.sl_hrname, dtc_fdsecdetcmd);
		fprintf(stderr, "detcon_server: Errno was: %d (may or may not be relevant).\n\n", save_errno);
		fflush(stderr);
		fprintf(dtc_fplog, 
			"detcon_server: at %s: connection CLOSED with hostname: %s with fd: %d for secondary detector control (temperature).\n", 
				dtc_ztime(), dtserver.sl_hrname, dtc_fdsecdetcmd);
		fprintf(dtc_fplog, "detcon_server: Errno was: %d (may or may not be relevant).\n\n", save_errno);
		fflush(dtc_fplog);

		if(NULL != dtc_connect_log)
		{
			fprintf(dtc_connect_log, 
				"detcon_server: at %s: connection CLOSED with hostname: %s with fd: %d for secondary detector control (temperature).\n", 
					dtc_ztime(), dtserver.sl_hrname, dtc_fdsecdetcmd);
			fprintf(dtc_connect_log, "detcon_server: Errno was: %d (may or may not be relevant).\n\n", save_errno);
			fflush(dtc_connect_log);
		}
		close(dtc_secdetport);
		dtc_secdetport = -1;
		return(0);
	}
	else
	{
		fprintf(stderr, 
			"detcon_server: at %s: Descriptor (%d) with unknown function given to nofity_server_eof.\n\n", 
				dtc_ztime(), fd);
		fflush(stderr);
		fprintf(dtc_fplog, 
			"detcon_server: at %s: Descriptor (%d) with unknown function given to nofity_server_eof.\n\n", 
				dtc_ztime(), fd);
		fflush(dtc_fplog);

		if(NULL != dtc_connect_log)
		{
			fprintf(dtc_connect_log, 
				"detcon_server: at %s: Descriptor (%d) with unknown function given to nofity_server_eof.\n\n", 
					dtc_ztime(), fd);
			fflush(dtc_connect_log);
		}
	        return(0);
	}
	return(0);
}

void	detcon_cleanexit(int status)
{
	fprintf(stderr,"detcon_server: shutting down with status %d\n",status);
	if(NULL != dtc_connect_log)
	{
		fprintf(dtc_connect_log, "detcon_server: at %s: Exiting via detcon_cleanexit (usally from a fatal network error).\n\n",
			dtc_ztime());
		fflush(dtc_connect_log);
		fclose(dtc_connect_log);
	}
	/*
	 *	Always exit with good status.
	 */
	exit(0);
}

/*
 *	Make connection with secondary (temperature control) inteface with det_api_workstation.
 *
 *	Returns:	0	No errors, and a connection is opened (on = 1) or closed (on = 0)
 *				properly.
 *
 *			1	An error.  Usually inability to connecto to det_api_workstation, or
 *				the environment is not properly set (secondary port numbers, etc.).
 */

int	detcon_server_enable_temperature_control(int on)
{
	char	buf[256];

	if(on && -1 != dtc_fdsecdetcmd)
		return(0);
	if(0 == on && -1 == dtc_fdsecdetcmd)
		return(0);

	sprintf(buf, "dtc_fdseccmd: %d dtc_secdetport: %d", dtc_fdsecdetcmd, dtc_secdetport);
	if(on)
		log_info("detcon_server_enable_temperature_control(ON)", buf, 0);
	else
		log_info("detcon_server_enable_temperature_control(OFF)", buf, 0);

	if(on)
	{
		if(-1 == dtc_secdetport)
			return(1);

		if(-1 != detcon_connect_to_host_api(&dtc_fdsecdetcmd, dtserver.sl_hrname, dtc_secdetport, NULL))
		{
			sprintf(buf, "connection OPEN with %s (fd: %d) for detector temperature control", 
					dtserver.sl_hrname,dtc_fdsecdetcmd);
			log_info("detcon_server", buf, 0);
			detcon_timestamp(dtc_fplog);
			fprintf(dtc_fplog,"detcon_server: connection established with hostname: %s with fd: %d for detector temperature control\n",
				dtserver.sl_hrname,dtc_fdsecdetcmd);
			fflush(dtc_fplog);
			if(NULL != dtc_connect_log)
			{
				fprintf(dtc_connect_log, 
					"detcon_server: at %s: connection OPENED with hostname: %s with fd: %d for detector temperature control.\n\n", 
						dtc_ztime(), dtserver.sl_hrname,dtc_fdsecdetcmd);
				fflush(dtc_connect_log);
			}
		}
		else
		{
			fprintf(stderr,"detcon_server: connection CANNOT be established with hostname: %s with fd: %d for detector temperature control\n",
				dtserver.sl_hrname,dtc_fdsecdetcmd);
			return(1);
		}
		dtc_temp_enabled = 1;
	}
	else
	{
		if(-1 == dtc_secdetport)
			return(1);

		sprintf(buf, "connection CLOSED with: %s (fd: %d) for detector temperature control", 
				dtserver.sl_hrname,dtc_fdsecdetcmd);
		log_info("detcon_server", buf, 0);
		detcon_timestamp(dtc_fplog);
		fprintf(dtc_fplog,"detcon_server: connection CLOSED with %s (fd: %d) for detector temperature control\n",
			dtserver.sl_hrname, dtc_fdsecdetcmd);
		fflush(dtc_fplog);
		if(NULL != dtc_connect_log)
		{
			fprintf(dtc_connect_log, 
				"detcon_server: at %s: connection CLOSED with hostname: %s with fd: %d for detector temperature control.\n\n", 
					dtc_ztime(), dtserver.sl_hrname, dtc_fdsecdetcmd);
			fflush(dtc_connect_log);
		}
		close(dtc_fdsecdetcmd);
		dtc_fdsecdetcmd = -1;
		dtc_temp_enabled = 0;
	}
	return(0);
}

/*
 *	Routine, called on its own thread, to handle connections and disconnections
 *	between the device library and det_api_workstation and ccd_image_gather.
 */

void	*detcon_server_update(void *arg)
{
	while(1)
	{
		detcon_check_other_servers();
		detcon_sleep(1000);
	}
}
