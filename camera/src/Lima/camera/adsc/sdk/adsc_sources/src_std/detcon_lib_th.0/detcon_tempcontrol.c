#include	"detcon_ext.h"

/*
 *	Enable/disable temperature control (on = 1/0).
 *
 *	on = 1:
 *		Beginning State:	DTC_STATE_IDLE
 *		Successful State:	DTC_STATE_IDLE
 *
 *	on = 0:
 *		Beginning State:	DTC_STATE_IDLE
 *		Successful State:	DTC_STATE_IDLE
 *
 *	Error State:			A function return of 1 *or* DTC_STATE_ERROR
 *					signals an error.
 *
 *	Errors are a bit tricky as if temperature control is tried while an exposure
 *	is in progress, the command is rejected but I don't want to change the state
 *	of the detector itself (which, in theory, is still OK, even though the control
 *	program made a mistate).
 *
 *	Returns:	0	No errors detected in this routine.
 *			1	An error is signaled in this routine
 *
 *				Errors would be considered "procedural" rather than "hardware"
 *				(result of an action).
 *
 *	The temperature control interface is automatically initialized on the first call
 *	to enable temperature regardless of the state of the "regular" device server library.
 *
 *	However, the regular device server interface SHOULD be initialized first if it is
 *	to be used to acquire data (some control systems will use this device library ONLY
 *	to control temperature).
 */

int	temp_enable_temperature_control(int on)
{
	int	state;
	char	buf[256];

	if(0 == dtc_initialized)
	{
		if(0 == dtc_sec_initialized)
		{
			detcon_server_init();
			detcon_ccd_initialize();
			strcpy(dtc_status_string,"Idle");
			strcpy(dtc_lasterror_string,"");
			dtc_sec_initialized = 1;
			setDTCState(DTC_STATE_IDLE, "in detcon_server_enable_temperature_control: Initial state (0 == dtc_initialized)", 0);
		}
	}

	if(DTC_STATE_IDLE != dtc_state && DTC_STATE_TEMPCONTROL != dtc_state)
	{
		sprintf(buf, "temp_enable_temperature_control: state not IDLE or TEMPCONTROL but %s\n", detcon_get_state_name(dtc_state));
		log_info("temp_enable_temperature_control(ON)", buf, 0);	
		return(1);
	}

	if(on)
	{
		state = detcon_server_enable_temperature_control(on);
		if(0 == state)
		{
			log_info("temp_enable_temperature_control(ON)", "OK from detcon_sever_enable_temperature_control(ON)", 0);	
			return(0);
		}
		else
		{
			log_info("temp_enable_temperature_control(ON)", "Error returned from detcon_server_enable_temperature(1)", 0);	
			return(1);
		}
	}
	else
	{
		state = detcon_server_enable_temperature_control(on);
		if(1 == state)
			log_info("temp_enable_temperature_control(OFF)", "Error returned from detcon_server_enable_temperature(0)", 0);	
		setDTCState(DTC_STATE_IDLE, "in temp_enable_temperature_control: IDLE with temperature control OFF.", 0);
		return(0);
	}
}

int	temp_read_temperature()
{
	if(DTC_STATE_TEMPCONTROL == dtc_state || DTC_STATE_IDLE == dtc_state)
		return(detcon_read_temperature());
	else
	{
		setDTCState(DTC_STATE_ERROR, "in temp_read_temperature: State not TEMPCONTROL or IDLE.", 0);
		return(1);
	}
}

int	temp_set_temperature(double val)
{
	char	msg[128];
	int	err;

	if(DTC_STATE_TEMPCONTROL != dtc_state && DTC_STATE_IDLE != dtc_state)
	{
		setDTCState(DTC_STATE_ERROR, "in temp_read_temperature: State not TEMPCONTROL.", 0);
		return(1);
	}

	if(val > 25. && val < -47.)
	{
		sprintf(msg, "temp_set_temperature: Invalid set value: %.2f", val);
		setDTCState(DTC_STATE_ERROR, msg, 0);
		return(1);
	}
	dtc_temp_value = val;

	sprintf(msg, "in temp_set_temperature(%.2f): state set to TEMPCONTROL", val);
	setDTCState(DTC_STATE_TEMPCONTROL, msg, 0);

	err = detcon_start_detached_thread((void *) &dtc_temperature_thread, detcon_set_temperature, NULL, "detcon_set_temperature");
	return(err);
}

int	temp_ramp_temperature(double val)
{
	int	err;
	char	msg[128];

	if(DTC_STATE_TEMPCONTROL != dtc_state && DTC_STATE_IDLE != dtc_state)
	{
		setDTCState(DTC_STATE_ERROR, "in temp_read_temperature: State not TEMPCONTROL.", 0);
		return(1);
	}

	if(val > 25. && val < -47.)
	{
		sprintf(msg, "temp_set_temperature: Invalid set value: %.2f", val);
		setDTCState(DTC_STATE_ERROR, msg, 0);
		return(1);
	}
	dtc_temp_value = val;

	sprintf(msg, "in temp_ramp_temperature(%.2f): state set to TEMPCONTROL", val);
	setDTCState(DTC_STATE_TEMPCONTROL, msg, 0);

	err = detcon_start_detached_thread((void *) &dtc_temperature_thread, detcon_ramp_temperature, NULL, "detcon_ramp_temperature");
	return(err);
}

int	temp_abort_temperature()
{
	char	msg[128];

	if(DTC_STATE_TEMPCONTROL != dtc_state)
	{
		sprintf(msg, "Warning: abort called with state not TEMPCON; was: %s\n", detcon_get_state_name(dtc_state));
		log_info("temp_abort_control", msg, 0);
		setDTCState(DTC_STATE_IDLE, "Return state to IDLE on abort.", 0);
		return(1);
	}
	return(detcon_abort_temperature());
}

/*
 *	These routines are called when parameters are set or retrieved from
 *	the hardware paramter routines in detcon_par.c.  This is the primary
 *	mechanism for setting or getting the detector temperature.
 *
 *	Temperature control is automacially enabled whenever these routines are called.
 */

int	detcon_temp_par_cold(int getset)
{
	if((0 != dtc_autosample_temp_per_image) && (0 == getset))
	{
		if(dtc_temps_transferred)
			return(0);
		else
			return(detcon_get_temperature_readings_fd(dtc_fddetcmd));
	}

	if(temp_enable_temperature_control(1))
		return(1);

	if(1 == getset)
	{
		dtc_temp_value = -45.;
		return(temp_ramp_temperature(dtc_temp_value));
	}

	return(0);
}

int	detcon_temp_par_warm(int getset)
{
	if((0 != dtc_autosample_temp_per_image) && (0 == getset))
	{
		if(dtc_temps_transferred)
			return(0);
		else
			return(detcon_get_temperature_readings_fd(dtc_fddetcmd));
	}
	if(temp_enable_temperature_control(1))
		return(1);

	if(1 == getset)
	{
		dtc_temp_value = 10.;
		return(temp_ramp_temperature(dtc_temp_value));
	}

	return(0);
}

int	detcon_temp_par_mode(int getset)
{
	return(0);
}

int	detcon_temp_par_status(int getset)
{
	if(dtc_autosample_temp_per_image)
	{
		if(dtc_temps_transferred)
			return(0);
		else
			return(detcon_get_temperature_readings_fd(dtc_fddetcmd));
	}
	if(temp_enable_temperature_control(1))
		return(1);

	if(1 == getset)
		return(1);

	return(detcon_get_temperature_readings_fd(dtc_fdsecdetcmd));
}

int	detcon_temp_par_value(int getset)
{
	if(temp_enable_temperature_control(1))
		return(1);

	if(1 == getset)
	{
		if(1 == dtc_temp_mode)
			return(temp_set_temperature(dtc_temp_value));
		else
			return(temp_ramp_temperature(dtc_temp_value));
	}

	return(temp_read_temperature());
}
