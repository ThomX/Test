#include	"detcon_ext.h"

/*
 *	Routines linking from detcon_entry.c
 *
 */

/*
 *	exec_ccd_start_exposure()
 *
 *
 *	Start an exposure.
 *
 *	Beginning State:	DTC_STATE_IDLE
 *	Intermediate State:	DTC_STATE_CONFIGDET	Detector is configuring.
 *	Successful State:	DTC_STATE_EXPOSING
 *
 *	Error State:		DTC_STATE_ERROR		Some kind of error.
 *
 *		The intermediate state CONFIGDET may or may not occur.
 *
 *		The error state ERROR will cause the reason to be logged and put
 *		into the CCDGetLastError() string and CCDStatus() string.
 *
 *		This routine can detect an asynchronously sent error from the
 *		ccd_image_gather process
 *
 *	Returns:	0	No errors detected in this routine.
 *			1	An error is signaled in this routine
 *
 *				Errors would be considered "procedural" rather than "hardware",
 *				(result of an action).
 */

int	exec_ccd_start_exposure()
{
	int 	err;
	char	buf[128];

	log_info("exec_ccd_start_exposure", "started", 0);	

	detcon_probe_other_servers();

	if(dtc_received_gather_error)
	{
		setDTCState(DTC_STATE_ERROR, "(in exec_ccd_start_exposure): received error from ccd_image_gather", 0);
		return(1);
	}

	if(dtc_initialized == 0)
	{
		setDTCState(DTC_STATE_ERROR, "exec_ccd_start_exposure: detcon_lib NOT initialized", 0);
		return(1);
	}
	if(dtc_state != DTC_STATE_IDLE)
	{
		sprintf(buf, "in exec_ccd_start_exposure: called with state %s; should be IDLE.", detcon_get_state_name(dtc_state));
		setDTCState(DTC_STATE_ERROR, buf, 0);
		return(1);
	}

	dtc_last = CCD_LAST_START;

	err = detcon_start_detached_thread((void *) &dtc_exec_thread, detcon_send_det_start, NULL, "exec_ccd_start_exposure");

	return(0);
}

/*
 *	exec_ccd_stop_exposure()
 *
 *
 *	Stop an exposure.
 *
 *	Beginning State:	DTC_STATE_EXPOSING
 *	Intermediate State:	DTC_STATE_READING	Detector is configuring.
 *	Successful State:	DTC_STATE_IDLE
 *
 *	Error State:		DTC_STATE_ERROR		Some kind of error.
 *
 *		The intermediate state READING always occurs but might be only for
 *		a short time interval.
 *
 *		The error state ERROR will cause the reason to be logged and put
 *		into the CCDGetLastError() string and CCDStatus() string.
 *
 *		This routine can detect an asynchronously sent error from the
 *		ccd_image_gather process
 *
 *	Returns:	0	No errors detected in this routine.
 *			1	An error is signaled in this routine
 *
 *				Errors would be considered "procedural" rather than "hardware",
 *				(result of an action).
 */

int	exec_ccd_stop_exposure()
{	
	int	err;
	char	buf[128];

	detcon_probe_other_servers();

	dtc_last = CCD_LAST_STOP;
	if(dtc_initialized == 0)
	{
		setDTCState(DTC_STATE_ERROR, "in exec_ccd_stop_image: detcon_lib NOT initialized", 0);
		return(1);
	}

	if(dtc_state != DTC_STATE_EXPOSING)
	{
		sprintf(buf, "in exec_ccd_stop_exposure: called with state %s; should be EXPOSING.", detcon_get_state_name(dtc_state));
		setDTCState(DTC_STATE_ERROR, buf, 0);
		return(1);
	}

	err = detcon_start_detached_thread((void *) &dtc_exec_thread, detcon_send_det_stop, NULL, "exec_ccd_stop_exposure");

	return(0);
}

/*
 *
 *	exec_ccd_get_image()
 *
 *	Cause the just collected image to be (eventually) written to disk by wrting
 *	a command to the ccd_image_gather process.
 *
 *	This is not a synchronous call; there is no way to wait for the image to
 *	be written to disk (at present).
 *
 *	Errors are detected at some future time, and not necesarily in this
 *	routine.
 *
 *	No change in the state is made other than indicating an error.
 *
 *	Returns:	0	No errors detected in this routine.
 *			1	An error is signaled in this routine
 *
 *				Errors would be considered "procedural" rather than "hardware",
 *				(result of an action).
 *
 */

int	exec_ccd_get_image(int last)
{
	detcon_probe_other_servers();

	if(dtc_received_gather_error)
	{
		setDTCState(DTC_STATE_ERROR, "(in exec_ccd_get_image): received error from ccd_image_gather", 0);
		return(1);
	}

	if(dtc_initialized == 0)
	{
		setDTCState(DTC_STATE_ERROR, "in exec_ccd_get_image: detcon_lib NOT initialized", 0);
		return(1);
	}
	dtc_last = CCD_LAST_GET;

	if(last == 0)
	{
		dtc_last = CCD_LAST_GET;
		if(detcon_send_copy_command())
		{
			setDTCState(DTC_STATE_ERROR, "in exec_ccd_get_image: received error from detcon_send_copy_command", 0);
			return(1);
		}
	}
	else if(last == 1)
	{
		dtc_last = CCD_LAST_IDLE;
		if(detcon_send_copy_command())
		{
			setDTCState(DTC_STATE_ERROR, "in exec_ccd_get_image: received error from detcon_send_copy_command", 0);
			return(1);
		}
		detcon_output_detcmd(dtc_fddetcmd,"flush\n",NULL,0);
	}
	else if(last == -1)
	{
		detcon_output_detcmd(dtc_fddetcmd,"flush\n",NULL,0);
		dtc_last = CCD_LAST_IDLE;
	}
	return(0);
}

/*
 *	exec_initialize()
 *
 *	Initialize the device server.
 *
 *	Set the state to IDLE.
 *
 *	No errors are (currently) signaled.
 *
 *	Returns:	0	No errors detected in this routine.
 *				No errors are (currently) signaled.
 */

int	exec_initialize()
{
	int	err;

	setDTCState(DTC_STATE_IDLE, "in exec_intialize: Initial state.", 0);

	dtc_received_gather_error = 0;

	detcon_server_init();
	detcon_ccd_initialize();

	if(((pthread_t) NULL) == dtc_server_thread)
		err = detcon_start_detached_thread( (void *) &dtc_server_thread, detcon_server_update, NULL, "detcon_server_update");

	strcpy(dtc_status_string,"Idle");
	strcpy(dtc_lasterror_string,"");

	dtc_initialized = 1;

	if(dtc_check_gather_return && ((pthread_t) NULL) == dtc_check_gather_return_thread)
		err = detcon_start_detached_thread( (void *) &dtc_check_gather_return_thread, detcon_check_gather_return, NULL, "detcon_check_gather_return");

	return(0);
}

/*
 *	exec_reset()
 *
 *	Reset the detector frame grabbers.  This is primarily a buffer reset.
 *
 *	Set the state to IDLE.
 *
 *	Returns:	0	No errors detected in this routine.
 *				No errors are (currently) signaled.
 */

int	exec_reset()
{
	detcon_probe_other_servers();

	detcon_output_detcmd(dtc_fddetcmd, "reset\n", NULL, 0);

	setDTCState(DTC_STATE_IDLE, "in exec_reset: Reset to Idle.", 0);

	dtc_received_gather_error = 0;

        strcpy(dtc_status_string,"Idle");
        strcpy(dtc_lasterror_string,"");

	return(0);
}

/*
 *	exec_hw_reset()
 *
 *	Reset the detector hardware.  This is primarily a reloading of the controllers,
 *	followed by a reset of the buffers.  Not a complete system reset.
 *
 *	Set the state to IDLE.
 *
 *	Returns:	0	No errors detected in this routine.
 *				No errors are (currently) signaled.
 */


int	exec_hw_reset()
{
	detcon_probe_other_servers();

	detcon_output_detcmd(dtc_fddetcmd, "hwreset\n", NULL, 0);

	setDTCState(DTC_STATE_IDLE, "in exec_hw_reset: Reset to Idle.", 0);

	dtc_received_gather_error = 0;

        strcpy(dtc_status_string,"Idle");
        strcpy(dtc_lasterror_string,"");

	return(0);
}

/*
 *	exec_ccd_abort()
 *
 *	Abort the current image in progress without the caller knowing what state
 *	the device server is in.
 *
 *	Set the state to IDLE.
 *
 *	This command takes the device server in any state and causes the current
 *	operation, if any, to be stopped immediately.  If no operation is in
 *	progress, nothing happens. If an image has been started, the image is
 *	guarenteed to be written to disk.  The device server is left in the IDLE
 *	state.
 *
 *	Returns:	0	Successful.
 *			1	An error occurred stopping an already started exposure,
 *				either from start or stop.
 *
 *				There are really no "procedural" errors possible; it is
 *				valid to call this routine at any time regardless of the
 *				state of the device server.
 */

int	exec_ccd_abort()
{
	detcon_probe_other_servers();

	switch(dtc_last)
	{
	case CCD_LAST_IDLE:
		setDTCState(DTC_STATE_IDLE, "in exec_ccd_abort: Reset to Idle (last state IDLE).", 0);
		break;
	case CCD_LAST_START:
		log_info("exec_ccd_abort (last state STARTED exposure)", "Entered, checking CCDState() for error or idle", 0);	
		while(DTC_STATE_EXPOSING != CCDState())
		{
			if(DTC_STATE_ERROR == CCDState())
			{
				log_info("exec_ccd_abort (last state STARTED exposure)", "Error returned from CCDStartExposure()", 0);	
				return(1);
			}
		}
		exec_ccd_stop_exposure();
		while(DTC_STATE_IDLE != CCDState())
		{
			if(DTC_STATE_ERROR == CCDState())
			{
				log_info("exec_ccd_abort (last state STARTED exposure)", "Error returned from CCDStopExposure()", 0);	
				return(1);
			}
		}
		switch(dtc_image_kind)
		{
		case 0:		/* first dark */
			if(dtc_output_raws)
				exec_ccd_get_image(1);
			break;
		case 4:
			if(dtc_output_raws)
				exec_ccd_get_image(1);
			break;
		case 1:		/* second dark or second(only) image */
		case 5:
			exec_ccd_get_image(1);
			break;
		}
		dtc_last = CCD_LAST_IDLE;
		break;
	case CCD_LAST_STOP:
		log_info("exec_ccd_abort (last state STOP exposure)", "Entered, calling exec_ccd_get_image(1)", 0);	
		exec_ccd_get_image(1);
		dtc_last = CCD_LAST_IDLE;
		break;
	case CCD_LAST_GET:
		log_info("exec_ccd_abort (last state GET_IMAGE)", "Entered, calling exec_ccd_get_image(-1)", 0);	
		exec_ccd_get_image(-1);
		dtc_last = CCD_LAST_IDLE;
		break;
	}
	return(0);
}
