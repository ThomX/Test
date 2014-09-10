#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<string.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<netdb.h>
#include	<errno.h>

int     probe_port_raw(int fd)
{
	fd_set		readmask;
	int		ret;
	struct timeval	timeout;
	char		cbuf;

        FD_ZERO(&readmask);
        FD_SET(fd,&readmask);
        timeout.tv_usec = 1000;
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
	if(1 != recv(fd, &cbuf, 1, MSG_PEEK))
		return(1);
	else
		return(-1);
}
