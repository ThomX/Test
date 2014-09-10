#include	"detcon_ext.h"

/*
 *	Routines to do I/O over sockets.
 */

static	int 	sio_string_found(buf,idex,ss)
char	*buf;
int	idex;
char	*ss;
{
	int	i,j,lss,bss;

	lss = strlen(ss);
	bss = idex - lss + 1;

	for(i = 0; i < bss ; i++)
	  {
	    for(j = 0; j < lss; j++)
	      if(ss[j] != buf[i + j])
		break;
	    if(j == lss)
		return(i);
	  }
	return(-1);
}

/*
 *	detcon_read_until:
 *
 *		Reads a maximum of maxchar from the file descriptor
 *		fd into buf.  Reading terminates when the string
 *		contained in looking_for is found.  Additional
 *		characters read will be discarded.
 *
 *		The read blocks.
 *
 *		For convienence, since we usually use this routine with
 *		string handling routines, scanf's, and such, the program
 *		will append a "null" to the end of the buffer.  The number
 *		of chars gets incremented by 1.
 *
 *	  Returns:
 *
 *		-1 on an error.
 *		number of characters read on sucess.
 *		0 on EOF.
 *
 *	Note:
 *
 *		This routine should not be used when it is expected that
 *		there may be more than one "chunk" of data present on
 *		the socket, as the subsequent "chunks" or parts thereof
 *		will be thrown away.
 *
 *		Ideal for situations where data is written to a process
 *		then one batch of info is written back, and no further data
 *		should be present until ADDITIONAL data is written to the
 *		process.
 */

int	detcon_read_until(int fd, char *buf, int maxchar, char *looking_for)
{
	int	eobuf,looklen,ret,utindex;
	fd_set	readmask;
	struct timeval	timeout;

	looklen = strlen(looking_for);
	utindex = 0;

	while(1)
	  {
		FD_ZERO(&readmask);
		FD_SET(fd,&readmask);
		timeout.tv_usec = 0;
		timeout.tv_sec = 1;
		ret = select(FD_SETSIZE, &readmask, (fd_set *) 0, (fd_set *) 0, NULL);
		if(ret == 0)
			continue;
		if(ret == -1)
		  {
		    if(errno == EINTR)
			continue;	/* interrupted system calls are OK. */
		    return(-1);
		  }
		if(0 == FD_ISSET(fd,&readmask))
			continue;
		ret = read(fd,&buf[utindex],maxchar - utindex);
		if(ret == -1)
			return(-1);
		if(ret == 0)
			return(0);

		utindex += ret;
		if(-1 != (eobuf = sio_string_found(buf,utindex,looking_for)))
		  {
			eobuf += looklen;
			buf[eobuf] = '\0';
			eobuf++;

			if(0)
			{
				fprintf(stderr,"------------ detcon_read_until START -----------------------------------------\n");
				fprintf(stderr,"%s", buf);
				fprintf(stderr,"------------ detcon_read_until END   -----------------------------------------\n");
			}
			return(eobuf);
		  }
		if(utindex == maxchar)
			return(utindex);
	  }
}

/*
 *	Function to do a write, with possible multiple chunks.
 *	We need this because of unknown buffering over the network.
 *
 *	The write blocks.
 *
 *	Returns the number of characters written, or -1 if an error.
 */

int	detcon_rep_write(int fd, char *buf, int count)
{
	char	*pos;
	int	remcount,i;

	if(count == 0)
		return(0);

	pos = buf;
	remcount = count;

	while(remcount > 0)
	  {
		i = write(fd,pos,remcount);
		if(i < 0)
		  {
		    detcon_timestamp(dtc_fplog);
		    fprintf(dtc_fplog,"detcon_rep_write: Error (%d) on file descriptor %d\n",errno,fd);
		    fflush(dtc_fplog);
		    perror("detcon_rep_write");
		    return(-1);
		  }
		remcount -= i;
		pos += i;
	  }
	return(count);
}

/*
 *	connect_to_host_api		connect to specified host & port.
 *
 *	Issue a connect to the specified host and port.  If the
 *	connection is sucessful, write the string (if non-null)
 *	msg over the socket.
 *
 *	If the operation is a sucess, returns the file descriptor
 *	for the socket via the fdnet pointer.  If this is -1, then
 *	the connection/message tranmission failed.
 *
 *	Also, returns the file descriptor if sucessful, otherwise
 *	-1.
 */

int	detcon_connect_to_host_api(int *fdnet, char *host, int port, char *msg)
{
	int	s;
	struct	sockaddr_in	server;
	unsigned int	len;
	struct	hostent	*hentptr,hent;
	char	localmsg[256];

	hentptr = gethostbyname(host);
	if(hentptr == NULL)
	  {
	    fprintf(stderr,"connect_to_host_api: no hostentry for machine %s\n",host);
	    *fdnet = -1;
	    return(*fdnet);
	  }
	hent = *hentptr;

	if(0)	/* DEBUG */
	  fprintf(stdout,"connect_to_host_api: establishing network connection to host %s, port %d\n",host,port);

	if(-1 == (s = socket(AF_INET, SOCK_STREAM, 0)))
	  {
		perror("connect_to_host_api: socket creation");
	    	*fdnet = -1;
		return(*fdnet);
	  }

	server.sin_family = AF_INET;
	server.sin_addr = *((struct in_addr *) hent.h_addr);
	server.sin_port = htons(port);

	if(connect(s, (struct sockaddr *) &server,sizeof server) < 0)
	  {
		*fdnet = -1;
		close(s);
		return(*fdnet);
	  }
	if(0)	/* DEBUG */
	  fprintf(stdout,"connect_to_host_api: connection established with host on machine %s, port %d.\n",host,port);
	if(msg != NULL)
	  {
	    strcpy(localmsg,msg);
	    len = strlen(msg);
	    localmsg[len] = '\0';
	    if(len > 0)
	      {
	      len++;
	      if(len != write(s,msg,len))
		{
		  fprintf(stdout,"connect_to_host_api: failure writing connect string (%s) command to host.\n",msg);
		  *fdnet = -1;
		  close(s);
		  return(*fdnet);
		}
	      }
	  }

	*fdnet = s;
	return(*fdnet);
}

/*
 *	Return 1 if the port has data, else 0.  Disconnects are discovered elsewhere.
 *	A disconnected port is marked as "done", i.e., 1.
 */

int	detcon_check_port_ready(int fd)
{
        fd_set  readmask;
        struct  timeval timeout;
        int     nb;
        char    buf[512];

        timeout.tv_sec = 0;
        timeout.tv_usec = 0;
        FD_ZERO(&readmask);
        FD_SET(fd,&readmask);
        nb = select(FD_SETSIZE, &readmask, (fd_set *) 0, (fd_set *) 0, &timeout);
        if(nb == -1)
          {
                if(errno == EINTR)
                  {
                    return(0);             /* timed out */
                  }
                fprintf(stderr,"detcon_check_port_ready: select error (on %d).  Should never happen.\n",fd);
                detcon_timestamp(dtc_fplog);
                fprintf(dtc_fplog,"detcon_check_port_ready: select error (on %d).  Should never happen.\n",fd);
                fflush(dtc_fplog);
                perror("detcon_check_port_ready: select in dt_check_fd");
                detcon_cleanexit(0);
          }
        if(nb == 0)
          {
                return(0);         /* no data ready */
          }
        if(0 == FD_ISSET(fd,&readmask))
          {
                return(0);         /* no data ready*/
          }

        nb = recv(fd,buf,512,MSG_PEEK);
        if(nb == 0)
          {
                return(0);
          }
	return(1);
}

/*
 *	read_port_raw:
 *
 *		Read data until there is none left.  Don't block.
 */

int	detcon_read_port_raw(int fd, char *stbuf, int stbufsize)
{
	int		nread;
	fd_set		readmask;
	int		ret;
	struct timeval	timeout;

	nread = 0;

	while(1)
	  {
	    FD_ZERO(&readmask);
	    FD_SET(fd,&readmask);
	    timeout.tv_usec = 0;
	    timeout.tv_sec = 0;
	    ret = select(FD_SETSIZE, &readmask, (fd_set *) 0, (fd_set *) 0, &timeout);
	    if(ret == 0)
			return(nread);
	    if(ret == -1)
	      {
			if(errno == EINTR)
		  	  continue;	/* Ignore interrupted system calls */
			detcon_notify_server_eof(fd, errno);
			return(-1);
	      }
	    if(0 == FD_ISSET(fd,&readmask))
	      {
			return(nread);
	      }
	    ret = recv(fd,stbuf + nread,stbufsize - nread,0);
	    if(ret == -1)
	      {
			if(errno == EINTR)
		  	continue;	/* Ignore interrupted system calls */
			detcon_notify_server_eof(fd, errno);
			return(-1);
	      }
	    if(ret == 0)
	      {
			detcon_notify_server_eof(fd, errno);
			return(-1);
	      }
	    nread += ret;
	  }
}
  
int	detcon_check_port_raw(int fd)
{
	fd_set		readmask;
	int		ret;
	struct timeval	timeout;

	FD_ZERO(&readmask);
	FD_SET(fd,&readmask);
	timeout.tv_usec = 0;
	timeout.tv_sec = 1;
	ret = select(FD_SETSIZE, &readmask, (fd_set *) 0, (fd_set *) 0, &timeout);
	if(ret == 0)
		return(0);
	if(ret == -1)
	  {
	    if(errno == EINTR)
		    return(0);		/* Ignore interrupted system calls */
		  else
		  	return(-1);
	  }
	 return(1);
}

int     detcon_probe_port_raw(int fd)
{
	fd_set		readmask;
	int		ret;
	struct timeval	timeout;
	char		cbuf;

        FD_ZERO(&readmask);
	FD_SET(fd,&readmask);
	timeout.tv_usec = 0;
	timeout.tv_sec = 0;
	ret = select(FD_SETSIZE, &readmask, (fd_set *) 0, (fd_set *) 0, &timeout);
	if(ret == 0)
		return(0);
	if(ret == -1)
	{
		if(errno == EINTR)
			return(0);          /* Ignore interrupted system calls */
		else
			return(-1);
	}
	if(0 == FD_ISSET(fd,&readmask))
	{
		return(0);         /* no data ready*/
	}
	if(1 != recv(fd,&cbuf,1,MSG_PEEK))
		return(-1);
	else
		return(1);
}

int     detcon_probe_port_raw_with_timeout(int fd, int nmicrosecs)
{
        fd_set		readmask;
        int             ret;
        struct timeval  timeout;
        char            cbuf;
	int		nsec;

	nsec = nmicrosecs / 1000000;
	nmicrosecs -= (nsec * 1000000);

        FD_ZERO(&readmask);
        FD_SET(fd,&readmask);
        timeout.tv_usec = nmicrosecs;
        timeout.tv_sec = nsec;
        ret = select(FD_SETSIZE, &readmask, (fd_set *) 0, (fd_set *) 0, &timeout);
        if(ret == 0)
                return(0);
        if(ret == -1)
          {
            if(errno == EINTR)
                    return(0);          /* Ignore interrupted system calls */
                  else
                        return(-1);
          }
         if(1 != recv(fd,&cbuf,1,MSG_PEEK))
                return(-1);
          else
                return(1);
}

/*
 *	Check for connection.  Return -1 if nothing pending, else return the
 *	accepted file descriptor.
 */

int	detcon_check_for_connection(int fd)
{
	struct	sockaddr_in	from;
	int 	g;
	unsigned int	len;
	int		nb;
	fd_set	readmask;
	struct	timeval	timeout;

	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	/*
	 *	Select for read the requested server socket
	 */

	FD_ZERO(&readmask);
	FD_SET(fd,&readmask);
	nb = select(FD_SETSIZE, &readmask, (fd_set *) 0, (fd_set *) 0, &timeout);
	if(nb == -1)
	  {
	  	if(errno == EINTR)
			return(-1);
		fprintf(stderr,"ccd_det_api: check_for_connection:: select error.  Should never happen.\n");
		fprintf(dtc_fplog,"ccd_det_api: check_for_connection:: select error.  Should never happen.\n");
		fflush(dtc_fplog);
		perror("ccd_det_api: check_for_connection:: select");
		detcon_cleanexit(0);
	  }
	if(nb == 0)
		return(-1);		/* nothing trying to connect */
	    
	/*
	 *	There is something to do.  If the listener socket is ready for read,
	 *	perform an accept on it.  If one of the others is ready to read, get
	 *	the data and output it to the screen.
	 */
	if(FD_ISSET(fd,&readmask))
	  {
	    len = sizeof from;
	    g = accept(fd, (struct sockaddr *) &from, &len);

	    if(g < 0)
	      {
	        if(errno != EINTR)
	          {
		    	fprintf(stderr,
		    		"ccd_det_api: check_for_connection:: accept error for network connection\n");
		    	fprintf(dtc_fplog,
		    		"ccd_det_api: check_for_connection:: accept error for network connection\n");
		    	fflush(dtc_fplog);
		    	perror("accept");
		    	detcon_cleanexit(0);
	          }
	      }
		return(g);
	  }
	return(-1);
}
