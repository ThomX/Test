#include	"detcon_ext.h"

/*
 *	This module contains logging related routines.
 */

static	char	*lit_detcon_states[] = {
					"IDLE        ",
					"EXPOSING    ",
					"READING     ",
					"ERROR       ",
					"CONFIGDET   ",
					"RETRY       ",
					"TEMPCONTROL ",
					NULL
				      };

static int	lit_detcon_nstates = sizeof lit_detcon_states;

char	*detcon_get_state_name(int state)
{
	if(state < 0 || state >= lit_detcon_nstates - 1)
		return("INVALID_STATE");
	else
		return(lit_detcon_states[state]);
}

/*
 *	Log a state change for reason reaason with system error errnum (from errno.h).
 *	"Success" will be logged if errnum is zero (no errror).
 */

void	detcon_log_state(int state, char *reason, int errnum)
{
	if(errnum > 0)
		fprintf(stdout,"detcon_log   : STAT:          at %s: state changed to %s reason: %s - %s\n",
			dtc_ztime(),lit_detcon_states[state], reason, strerror(errnum));
	else
		fprintf(stdout,"detcon_log   : STAT:          at %s: state changed to %s reason: %s\n",
			dtc_ztime(),lit_detcon_states[state], reason);
	if(DTC_STATE_ERROR == state)
	{
		strcpy(dtc_status_string, reason);
		strcpy(dtc_lasterror_string, reason);
	}
	fflush(stdout);
}

/*
 *	Log information with further information further with system error errnum (from errno.h).
 *	"Success" will be logged if errnum is zero (no errror).
 */

void	log_info(char *info, char* further, int errnum)
{
	if(errnum > 0)
		fprintf(stdout,"detcon_log   : INFO:          at %s:                              message: %s-%s - %s\n",
			dtc_ztime(), info, further, strerror(errnum));
	else
		fprintf(stdout,"detcon_log   : INFO:          at %s:                              message: %s-%s\n",
			dtc_ztime(), info, further);
	fflush(stdout);
}
