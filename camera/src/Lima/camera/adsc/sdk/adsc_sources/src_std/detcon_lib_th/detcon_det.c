#include	"detcon_ext.h"

static  char    timeholder[120];

char    *dtc_ztime()
  {
        long    clock;
        char    *cptr;

        time(&clock);
        cptr = (char *) ctime(&clock);
        strcpy(timeholder,cptr);
        timeholder[strlen(timeholder) - 1] = '\0';
        return(timeholder);
  }

/*
 *	Handle communication to/from detector control process.
 */

/*
 *	Output a detector command line.
 *
 *	If the descriptor is not defined, forget it.  If the
 *	beam line command buffer does not contain anything for
 *	this run, don't output anything.
 *
 *	The second argument is a string, which can be null, to
 *	be prepended onto the beam line command argument (third
 *	argument).  This is how the detector control program
 *	can distinguish between "before", "during", and "after"
 *	runs (in case it cares).
 *
 *	If the string is sucessfully written, we read the socket.
 *	When data is present, or an error occurs, we are done.
 *
 *	This causes a stall in the execution of ccd_dc.  The
 *	status should continue to update while this occurs.
 */

static	char	*detterm = "end_of_det\n";

static	char	det_outbuf[20480];

int	detcon_output_detcmd(int fd, char *detcmd, char *hdptr, int hdsize)
{
	char	tbuf[512],temp_xx[512],*retstr;
	int	i,len,ret,detret;

	detret = CCD_DET_OK;
	retstr = "normal";

	if(fd == -1)
		return(CCD_DET_NOTCONNECTED);

	if(detcmd[0] == '\0')
		return(CCD_DET_OK);

	strcpy(det_outbuf,detcmd);
	strcat(det_outbuf,detterm);

	if(0)
	  fprintf(stdout,"output_detcmd: sending:\n==========\n%s===========\n",det_outbuf);
	 else
	  {
	    if(detcmd[0] != '\0')
	      {
		sscanf(det_outbuf,"%s",temp_xx);
		fprintf(stdout,"detcon_output: SENT:          at %s: %s\n",dtc_ztime(),temp_xx);
		fflush(stdout);
	      }
	  }

	len = strlen(det_outbuf);
	for(i = 0; i < hdsize; i++,len++)
	    det_outbuf[len] = hdptr[i];

	ret = detcon_rep_write(fd,det_outbuf,len);
	if(ret <= 0)
	  {
	    detcon_notify_server_eof(fd, errno);
	    return(CCD_DET_DISCONNECTED);
	  }

	/*
	 *	Now check the socket for data present.
	 */

	ret = detcon_read_until(fd,dtc_det_reply,sizeof dtc_det_reply,detterm);

	if(ret <= 0)
	  {
	        detcon_notify_server_eof(fd, errno);
	        return(CCD_DET_DISCONNECTED);
	  }

	sscanf(dtc_det_reply,"%s",tbuf);
	if(0)
	  fprintf(stdout,"output_detcmd: ccd_det returned: %s\n",tbuf);
	if(0 == strcmp("RETRY1",tbuf))
	  {
		detret = CCD_DET_RETRY;
		retstr = "retry";
          }
        else if(0 == strcmp("RETRY2",tbuf))
          {
                detret = CCD_DET_RETRY;
                retstr = "retry";
          }
        else if(0 == strcmp("RETRY3",tbuf))
          {
                detret = CCD_DET_RETRY;
                retstr = "retry";
          }
        else if(0 == strcmp("RETRY",tbuf))
          {
                detret = CCD_DET_RETRY;
                retstr = "retry";
	  }
	 else
	  {
	    if(0 == strcmp("ERROR",tbuf))
	      {
		detret = CCD_DET_FATAL;
		retstr = "fatal";
	      }
	  }
	fprintf(stdout,"detcon_output: DONE: (%s) at %s: %s\n",retstr,dtc_ztime(),temp_xx);
	fflush(stdout);
	return(detret);
}

/*
 *	Issue the command and return.  Data ready on the return from this command
 *	will be checked elsewhere.
 */

int	detcon_output_detcmd_issue(int fd, char *detcmd, char *hdptr, int hdsize)
{
	char	temp_xx[512],*retstr;
	int	i,len,ret,detret;

	detret = CCD_DET_OK;
	retstr = "normal";

	if(fd == -1)
		return(CCD_DET_NOTCONNECTED);

	if(detcmd[0] == '\0')
		return(CCD_DET_OK);

	strcpy(det_outbuf,detcmd);
	strcat(det_outbuf,detterm);

	if(0)
	{
	    if(detcmd[0] != '\0')
	      {
		sscanf(det_outbuf,"%s",temp_xx);
		fprintf(stdout,"detcon_issue : SENT:          at %s: %s\n",dtc_ztime(),temp_xx);
	      }
	  fprintf(stdout,"output_detcmd: sending:\n==========\n%s===========\n",det_outbuf);
	}
	 else
	  {
	    if(detcmd[0] != '\0')
	      {
		sscanf(det_outbuf,"%s",temp_xx);
		fprintf(stdout,"detcon_issue : SENT:          at %s: %s\n",dtc_ztime(),temp_xx);
		fflush(stdout);
	      }
	  }

	len = strlen(det_outbuf);
	for(i = 0; i < hdsize; i++,len++)
	    det_outbuf[len] = hdptr[i];

	ret = detcon_rep_write(fd,det_outbuf,len);
	if(ret <= 0)
	  {
	    detcon_notify_server_eof(fd, errno);
	    return(CCD_DET_DISCONNECTED);
	  }

	return(CCD_DET_OK);
}

/*
 *	Once data ready on the detector socket has been determined, this
 *	routine will be called to receive the return info.
 */

int	detcon_output_detcmd_receive(int fd)
{
	char	tbuf[512],*retstr;
	int	i,ret,detret;
	char	*cp, *cp1, *cp2;

	/*
	 *	Now check the socket for data present.
	 */

	ret = detcon_read_until(fd,dtc_det_reply,sizeof dtc_det_reply,detterm);

	if(ret <= 0)
	  {
	        detcon_notify_server_eof(fd, errno);
	        return(CCD_DET_DISCONNECTED);
	  }

	retstr = "normal";
	detret = CCD_DET_OK;
	if(NULL != (cp1 = strstr(dtc_det_reply, "__command_status__")))
	{
		/*
		 *	New style return information
		 */
		cp2 = strstr( cp1 + 1, "__command_status__");

		for(i = 0, cp = cp1 + strlen("__command_status__"); cp < cp2; cp++, i++)
			tbuf[i] = *cp;
		tbuf[i + 1] = '\0';

		if(NULL != (cp2 = strstr(dtc_det_reply, detterm)))
			*cp2 = '\0';
		else
			fprintf(stdout,"WARNING: No end_of_det found in buffer %s\n", dtc_det_reply);
	}
	else
	{
		sscanf(dtc_det_reply, "%s", tbuf);
	}
//	if(1)
//		fprintf(stderr,"detcon_recv'd: INFO          at %s: detector returned %s\n",dtc_ztime(),tbuf);
//		fprintf(stderr,"detcon_recv'd: INFO          at %s: detector returned %s\n",dtc_ztime(),dtc_det_reply);
	if(0 == strcmp("RETRY1",tbuf))
	  {
		detret = CCD_DET_RETRY;
		retstr = "retry ";
          }
        else if(0 == strcmp("RETRY2",tbuf))
          {
                detret = CCD_DET_RETRY;
                retstr = "retry ";
          }
        else if(0 == strcmp("RETRY3",tbuf))
          {
                detret = CCD_DET_RETRY;
                retstr = "retry ";
          }
        else if(0 == strcmp("RETRY",tbuf))
          {
                detret = CCD_DET_RETRY;
                retstr = "retry ";
	  }
	 else
	  {
	    if(0 == strcmp("ERROR",tbuf))
	      {
		detret = CCD_DET_FATAL;
		retstr = "fatal ";
	      }
	  }
	fprintf(stdout,"detcon_recv'd: DONE: (%s) at %s\n",retstr,dtc_ztime());
	fflush(stdout);
	return(detret);
}

int	detcon_output_gather_receive(int fd)
{
	char	tbuf[512],*retstr;
	int	ret,detret;
	/*
	 *	Now check the socket for data present.
	 */

	ret = detcon_read_until(fd,dtc_xform_reply,sizeof dtc_xform_reply,detterm);

	if(ret <= 0)
	{
	        detcon_notify_server_eof(fd, errno);
	        return(CCD_DET_DISCONNECTED);
	}

	retstr = "normal";
	detret = CCD_DET_OK;
	sscanf(dtc_xform_reply,"%s",tbuf);
	if(1)
	{
		fprintf(stdout," xform_recv'd: RTRN:          at %s: %s\n",dtc_ztime(),retstr);
		fflush(stdout);
	}
	if(NULL != strstr(dtc_xform_reply, "RETRY"))
	{
		detret = CCD_DET_RETRY;
		retstr = "retry ";
	}
	else
	{
		if(NULL != strstr(dtc_xform_reply, "ERROR"))
		{
			detret = CCD_DET_FATAL;
			retstr = "fatal ";
		}
	}
	fprintf(stdout," xform_recv'd: DONE: (%s) at %s\n",retstr,dtc_ztime());
	fflush(stdout);
	return(detret);
}
