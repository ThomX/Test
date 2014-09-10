#include	"detcon_ext.h"

/*
 *	All thread and mutex related code should go in this module.
 *
 *	Changing between pthreads threads and mutexes and OmniNotify
 *	threads and mutexes should be #ifdef'ed here.
 */

#ifdef USE_PTHREADS

/*
 *	Set a new state to newState for reason reason with system error (from errno.h).
 *	"Success" will be logged if errnum is zero.
 */

void setDTCState(int newState, char* reason,int errnum)
{
	pthread_mutex_lock(&dtc_h_mutex_state);
	dtc_state = newState;
	pthread_mutex_unlock(&dtc_h_mutex_state);
	detcon_log_state(newState, reason, errnum);
}

/*
 *	Start a detached thread with function fcn which is of the form:
 *
 *		int	fcn(void *arg);
 *
 *	Handle is a pointer to a thread handle. Using pthreads, it will be a pointer
 *	to a variable of type pthread_t.
 *
 *	logging_tag is a character string used for logging that, for example, would identify
 *	which thread is being started.
 *
 *	On success, 0 is returned.  If any error is encountered it is logged and 1 is returned.
 */

int	detcon_start_detached_thread(void *handle, void*(fcn)(), void *arg, char *logging_tag)
{
	pthread_attr_t	attr;
	int		err;
	pthread_t	*pt;

	pt = (pthread_t *) handle;

	log_info(logging_tag, "before pthread_create",0);	
	err = pthread_attr_init(&attr);
	if(err != 0){
		char	buf[1024];
		sprintf(buf, "%s - pthread_attr_init failed", logging_tag);
		setDTCState(DTC_STATE_ERROR, buf ,errno);
		return( 1);
	}
	err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	if(err != 0){
		char	buf[1024];
		sprintf(buf, "%s - pthread_attr_setdetechstate failed", logging_tag);
		setDTCState(DTC_STATE_ERROR, buf ,errno);
		pthread_attr_destroy(&attr);
		return( 1);
	}
	err = pthread_create(pt, &attr, fcn, NULL);
	if(err != 0)
	{
		char	buf[1024];
		sprintf(buf, "%s - pthread_create failed", logging_tag);
		setDTCState(DTC_STATE_ERROR, buf ,errno);
		pthread_attr_destroy(&attr);
		return( 1);
	}
	pthread_attr_destroy(&attr);
	return(0);
}

#endif /* USE_PTHREADS */

#ifdef USE_OMNI

/*
 *	We need to put in the appropriate code which uses the OmniNotify threads
 *	and mutexes.
 */

#endif /* USE_OMNI */
