#include	"ext.h"
#include	<math.h>
#include	<time.h>

static int debug_img_no = 0;

char                    timecopy[256];
time_t			time_rec_start;
time_t			time_xform_end;

int		begin_recv, end_recv, end_disk_write;
int		begin_repack, end_repack;
float		found_time = 0;
int		fdadxv = -1;

void	send_to_adxv_socket(char *fname)
{
	int	port;
	char	*cp_host, *cp_port;
	char	buf[512];

	if(NULL == (cp_host = getenv("ADXV_HOSTNAME")))
		return;
	if(NULL != (cp_port = getenv("ADXV_DISPLAY_PORT")))
		port = atoi(cp_port);
	else
		port = 8100;

	if(-1 == fdadxv)
	{
		if(-1 == (fdadxv = connect_to_host(&fdadxv, cp_host, port, NULL)))
		{
			return;
		}
	}

	sprintf(buf,"load_image %s\n",fname);
	if(-1 == probe_port_raw(fdadxv))
	{
		close(fdadxv);
		fdadxv = -1;
		return;
	}
	if(-1 == rep_write(fdadxv, buf, strlen(buf)))
	{
		close(fdadxv);
		fdadxv = -1;
		return;
	}
}

static char	*image_type_str[] = {
					"dkx_0",
					"dkx_1",
					"dkx_2",
					"dkx_3",
					"raw_0",
					"raw_1",
				    };
char    *ztime()
  {
        time_t  tval;
        char    *cstr;

        time(&tval);
        cstr =  (char *) ctime(&tval);
        strcpy(timecopy,cstr);
        timecopy[strlen(timecopy) - 6] = '\0';
        return(&timecopy[4]);
  }


void	dump_it(var, xsize, ysize, dump_name, im_no)
unsigned short	*var;
char		*dump_name;
int		xsize, ysize, im_no;
  {
	FILE	*fpjunk;
	char 	debug_file_name[132];
	char	debug_header[512];
	int	debug_len;

	clrhd(debug_header);
	puthd("DIM","2",debug_header);
#if defined(alpha) || defined(linux)
        puthd("BYTE_ORDER","little_endian",debug_header);
#else
	puthd("BYTE_ORDER","big_endian",debug_header);
#endif /* alpha */
        puthd("TYPE","unsigned_short",debug_header);
	sprintf(debug_file_name,"%d",xsize);
	puthd("SIZE1",debug_file_name,debug_header);
	sprintf(debug_file_name,"%d",ysize);
	puthd("SIZE2",debug_file_name,debug_header);
	padhd(debug_header,512);
	gethdl(&debug_len,debug_header);


	sprintf(debug_file_name,"%s_%d.dbx",dump_name,im_no);
	if(NULL != (fpjunk = fopen(debug_file_name,"w")))
	  {
	    fwrite(debug_header,sizeof (unsigned char), debug_len, fpjunk);
	    fwrite(var,sizeof (unsigned short), xsize * ysize, fpjunk);
	    fclose(fpjunk);
	  }
	 else
	  fprintf(stdout,"dump_it: (info only, not an error):could not open or create %s\n",debug_file_name);
  }

/*
 *	repack_memory(in, rotraw, ptr)
 *
 *	Rearrange transmitted image to A B
 *				       C D
 *
 *	From A
 *	     B
 *	     C
 *	     D
 *
 *	or 0 1 2 3 4 5 6 7 8
 *		into
 *	0 1 2
 *	3 4 5
 *	6 7 8
 *
 */

void	repack_memory(in, out, quad_size)
unsigned short 	*in, *out;
int		quad_size;
  {
	int		ss,i,j,m,n,collen;
	int		sqrt_n_ctrl;
	unsigned short	*pfrom, *pto;

	if(n_ctrl == 1)
		sqrt_n_ctrl = 1;
	else if(n_ctrl == 4)
		sqrt_n_ctrl = 2;
	else if(n_ctrl == 9)
		sqrt_n_ctrl = 3;
	else sqrt_n_ctrl = 0;

	ss = quad_size / det_bin;
	collen = ss * sqrt_n_ctrl;

	if(0)
	{
	    dump_it(in, collen / det_bin, collen / det_bin, "in", debug_img_no);
	    debug_img_no++;
	}

	/*
	 *	This repacks.
	 *
	 *	Packs A over B into A | B, then below that C Over D to C | D
	 *
	 *	or 0 1 2 3 4 5 6 7 8
	 *
	 *		into
	 *
	 *		0 1 2
	 *		3 4 5
	 *		6 7 8
	 */

	if(0)
	{
	for(n = 0; n < sqrt_n_ctrl; n++)
	{
	    pto = scratch;
	    pfrom = in + n * sqrt_n_ctrl * ss * ss;
	    for(m = 0; m < sqrt_n_ctrl; m++)
	    {
	    	for(i = 0; i < ss; i++)
	    		for(j = 0; j < ss; j++)
				*(pto + i * collen + j + m * ss) = *pfrom++;
	    }
	    pfrom = scratch;
	    pto = out + n * sqrt_n_ctrl * ss * ss;

	    for(i = 0; i < ss; i++)
	    for(j = 0; j < collen; j++)
		*pto++ = *pfrom++;
	}
	}
	else
	{
	for(n = 0; n < sqrt_n_ctrl; n++)
	{
	    pto = scratch;
	    pfrom = in + n * sqrt_n_ctrl * ss * ss;
	    for(m = 0; m < sqrt_n_ctrl; m++)
	    {
	    	pfrom = in + (n * sqrt_n_ctrl + m) * ss * ss;
	    	for(i = 0; i < ss; i++)
		{
			pto = out + n * sqrt_n_ctrl * ss * ss + i * collen + m * ss;;
	    		for(j = 0; j < ss; j++)
				*pto++ = *pfrom++;
		}
	    }
	}
	}

	if(0)
	{
	    dump_it(out, collen / det_bin, collen / det_bin, "rr", debug_img_no);
	    debug_img_no++;
	}

        if(detector_sn != -1)
        {
            switch(detector_sn)
            {
                default:
                  break;
            }
        }
  }	

/*
 *	Probe a file to see if it is ready to read.
 *
 *	Return 0 if no data.
 *	Return 1 if data is present after a peek.
 *	Return -1 if an error.
 */

int     probe_port_raw(int fd)
  {
        fd_set		readmask;
        int		ret;
        struct timeval  timeout;
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
	        {
		  return(-1);
		}
          }
	if(1 != recv(fd,&cbuf,1,MSG_PEEK))
	  {
	    return(-1);
	  }
          else
	    return(1);
  }

/*
 *	Function to do a read from multiple file descriptors, servicing
 *	each as they have data to read, and making sure that an adquate
 *	time slice is given to each so that the data are drained from the
 *	frame grabber PCs in roughly the same time in order to preserve
 *	parallelism.
 */

int	rep_read_mult(fds,nfds,hbuf,dbuf,hcount,dcount)
int	fds[],nfds,hcount,dcount;
char	*hbuf,*dbuf;
  {
	char	*pos[MAX_CONTROLLERS];
	int	remcount[MAX_CONTROLLERS];
	int	fstates[MAX_CONTROLLERS];
	int	i,n,res,completed_fds,got_error;

	fprintf(stderr,"rep_read_mult: nfds: %d ",nfds);
	fprintf(stderr,"fds: ");
	for(i = 0; i < nfds; i++)
	  fprintf(stderr," %2d ", fds[i]);
	fprintf(stderr," hcount: %d with dcount: %d\n",hcount, dcount);
	/*
	 *	First initialize remcount to header counts, and buffer pointers
	 *	to the header pointer.  We rely on the fact that all headers
	 *	are identical for "reusing" the header buffer pointer.
	 */

	for(n = 0; n < nfds; n++)
	  {
		remcount[n] = hcount;	/* header counts */
		pos[n] = hbuf;		/* points to header buffer */
		fstates[n] = -1;	/* indicate looking for header */
	  }

	/*
	 *	Loop over available file descriptors.  This is indicated by
	 *	fstates[n] < cstates[n].
	 */

	got_error = 0;
	completed_fds = 0;
	while(completed_fds < nfds)
	  {
	    for(n = 0; n < nfds; n++)
	      if(fstates[n] < q_states[n])
	        {
	    	      res = probe_port_raw(fds[n]);
		      if(res == -1)	/* error */
		        {
		  	  fprintf(stderr,"rep_read_mult: Error probing for available data, detector process %d\n",n);
		  	  notify_server_eof(fds[n], errno);
			  fstates[n] = q_states[n];
			  got_error = 1;
			  continue;
		        }
		      if(res == 0)	/* no data */
			continue;
		/*
		 *	Data is present.  Try to read all the remaining bytes for this
		 *	segment of the socket or file.
		 */
		res = read(fds[n], pos[n], remcount[n]);

		if(res <= 0)	/* error or EOF */
		  {
		  	fprintf(stderr,"rep_read_mult: Error reading detector process %d\n",n);
			perror("reading detector process");
			fprintf(stderr,"fds[%d]: %d remcount[%d] %d pos[%d]: %d\n", n, fds[n], n, remcount[n], n, (int) pos[n]);
		  	notify_server_eof(fds[n], errno);
			fstates[n] = q_states[n];
			got_error = 1;
			continue;
		  }
		remcount[n] -= res;	/* decrement count this file descriptor */
		pos[n] += res;		/* increment buffer pointer this descriptor */

		if(remcount[n] == 0)
		  {
		    fstates[n]++;

		    if(fstates[n] == q_states[n])
		      {
		        completed_fds++;
		    	continue;	/* we are done with this file */
		      }
		    pos[n] = dbuf + q_blocks[n][fstates[n]] * dcount;
		    remcount[n] = dcount;
		  }
	        }
	  }

	return(got_error);
  }

/*
 *	Function to do a read from multiple file descriptors, servicing
 *	each as they have data to read, and making sure that an adquate
 *	time slice is given to each so that the data are drained from the
 *	frame grabber PCs in roughly the same time in order to preserve
 *	parallelism.
 *
 *	In this version, a variable header is read AND the actual length
 *	of the read header (the headers of multiple frame grabber sequences
 *	of modules better be basically the same) is returned in hcountp.
 */

char	hbstring[] = "HEADER_BYTES";

int	rep_read_mult_var_header(fds,nfds,hbuf,dbuf,hcountp,dcount)
int	fds[],nfds,*hcountp,dcount;
char	*hbuf,*dbuf;
  {
	char	*pos[MAX_CONTROLLERS];
	int 	remcount[MAX_CONTROLLERS];
	int 	fstates[MAX_CONTROLLERS], cstates[MAX_CONTROLLERS];
	int 	first_hdreads[MAX_CONTROLLERS];
	int 	i,n,hbf,res,completed_fds,got_error;
	char	field[20];
	int 	act_hlen;
	char	*tstr;

	if(0)
	{
		fprintf(stderr,"rep_read_mult_var_header: nfds: %d ",nfds);
		fprintf(stderr,"fds: ");
		for(i = 0; i < nfds; i++)
			fprintf(stderr," %2d ", fds[i]);
		fprintf(stderr," with dcount: %d\n", dcount);
	}
	/*
	 *	First initialize remcount to header counts, and buffer pointers
	 *	to the header pointer.  We rely on the fact that all headers
	 *	are identical for "reusing" the header buffer pointer.
	 */

	for(n = 0; n < nfds; n++)
	{
		remcount[n] = 512;	/* header counts  DEBUG:  THIS IS A KLUDGE */
		*hcountp = 512;
		pos[n] = hbuf;		/* points to header buffer */
		fstates[n] = -1;	/* indicate looking for header */
		cstates[n] = 1;
		first_hdreads[n] = 1;
	}

	/*
	 *	Loop over available file descriptors.  This is indicated by
	 *	fstates[n] < cstates[n] for this version, we set cstates[n]
	 *	equal to 1 so that just the header and data for each module
	 *	is read thru a single socket.  The more elaborate scheme
	 *	was present in the version where the header + all module
	 *	data (1 or more) was sent thru the (only) data socket.
	 */

	got_error = 0;
	completed_fds = 0;
	while(completed_fds < nfds)
	{
	    for(n = 0; n < nfds; n++)
		if(fstates[n] < cstates[n])
	        {
	    	        res = probe_port_raw(fds[n]);
		        if(res == -1)	/* error */
		        {
		 		fprintf(stderr,
				  "rep_read_mult: Error probing for available data, module number %d fd: %d\n",
				  		n, fds[n]);
		 		notify_server_eof(fds[n], errno);
				fstates[n] = cstates[n];
				got_error = 1;
				continue;
			}
			if(res == 0)	/* no data */
				continue;
			/*
			 *	Data is present.  Try to read all the remaining bytes for this
			 *	segment of the socket or file.
			 */
			res = read(fds[n], pos[n], remcount[n]);

			if(res <= 0)	/* error or EOF */
			{
		  		fprintf(stderr,"rep_read_mult: Error reading module number %d, fd: %d\n",n, fds[n]);
				perror("reading detector process");
				fprintf(stderr,
					"fds[%d]: %d remcount[%d] %d pos[%d]: %d\n", n, fds[n], n, remcount[n], n, (int) pos[n]);
		  		notify_server_eof(fds[n], errno);
				fstates[n] = cstates[n];
				got_error = 1;
				completed_fds++;
				continue;
			}
			if(first_hdreads[n] == 1)
			{
				/*
				 *	Replace the gethd with a check and extract.
				 *	This will fail big time if the header is messed up.
				 */
				hbf = 1;
				for(i = 2; i < 12; i++)
				  if(*(pos[n] + i) != hbstring[i-2])
				  	hbf = 0;
				if(hbf == 0)
				{
					fprintf(stderr,
					"rep_read_mult_var_header: ERROR: HEADER_BYTES string not found mod: %d\n",
							n);
					act_hlen = 512;
				}
				else
				{
					for(i = 15; i < 20; i++)
						field[i-15] = *(pos[n]+i);
					field[5] = '\0';
					act_hlen = atoi(field);
				}
				*hcountp = act_hlen;
				remcount[n] += (act_hlen - 512);
				first_hdreads[n] = 0;
				if(0)
				fprintf(stderr,"rep_read_mult_var_header: fd: %d, act_hlen: %d new remcount[%d]: %d\n",
					n, act_hlen, n, remcount[n]);
				if(n == 0)
				{
					tstr = ztime();
					fprintf(stdout,"image_gather:  STRT          at %s: image xfer begun.\n",tstr); 
					fflush(stdout);
					begin_recv = timeGetTime();
				}
			} 
			remcount[n] -= res;	/* decrement count this file descriptor */
			pos[n] += res;		/* increment buffer pointer this descriptor */

			if(remcount[n] == 0)
		  	{
		    		fstates[n]++;

		    		if(fstates[n] == cstates[n])
		      		{
		        		completed_fds++;
		    			continue;	/* we are done with this file */
		      		}
		    		pos[n] = dbuf + n * dcount;
		    		remcount[n] = dcount;
		  	}
		}
	}

	return(got_error);
}

/*
 *	Handles the reception of raw images to this gathering proecss
 *
 *	Changes for multiple frame grabber PCs.  We will be receiving data from multiple
 *	sockets containing the data.  Each frame grabber PC will send a header and its
 *	data.  The header from each frame grabber PC is identical.
 */

int	receive_data_raw(int ind)
  {
	char		*tstr;
	unsigned short	*datap;
	int		size_of_quadrant;
	char    	field[132];
	int		read_fds[MAX_CONTROLLERS];
	int		n,n_fds;
	double		atof();

	for(n = 0; n < n_ctrl; n++)
    		read_fds[n] = data_fd[n];
	n_fds = n_ctrl;

	tstr = ztime();
	fprintf(stdout,"image_gather:  WAIT          at %s: image_type:   %s\n",tstr, image_type_str[image_kind]); 
	fflush(stdout);
	begin_recv = timeGetTime();
	  
	/*
	 *	If the data comes from the detector api, it will be a multiple of ccd_sqsize.
	 */

	size_of_quadrant = (ccd_sqsize * ccd_sqsize * sizeof (unsigned short)) / (det_bin * det_bin);

	if(ind >= KIND_RAW_0)
	{
		(void) rep_read_mult_var_header(read_fds, n_fds, raw_header[ind - KIND_RAW_0], in_data, 
					&raw_header_size[ind - KIND_RAW_0], size_of_quadrant);
		gethd("TIME", field, raw_header[ind - KIND_RAW_0]);
	}
	else
	{
		(void) rep_read_mult_var_header(read_fds, n_fds, dkc_header[ind], in_data, 
					&dkc_header_size[ind], size_of_quadrant);
		gethd("TIME", field, raw_header[ind]);
	}

	found_time = atof(field);
	  
	time(&time_rec_start);
	tstr = ztime();
	end_recv = timeGetTime();
	fprintf(stdout,"image_gather:  RCVD          at %s: received in: %6.3f secs.\n",tstr, 
			(end_recv - begin_recv) / 1000.); 
	fflush(stdout);
	if(fpg != NULL)
	{
		fprintf(fpg,"image_gather:  RCVD          at %s: received in: %6.3f secs.\n",tstr, 
			(end_recv - begin_recv) / 1000.); 
		fflush(fpg);
	}

	if(0)
	{
		dump_it(in_data, (ccd_sqsize * n_ctrl) / (2 * det_bin), (ccd_sqsize * n_ctrl) / (2 * det_bin), 
					"file_imin", debug_img_no);
	    	debug_img_no++;
	}

	if(ind >= KIND_RAW_0)
		datap = raw_data[ind-KIND_RAW_0];
	else
		datap = dkc_data[ind];

	begin_repack = timeGetTime();
	repack_memory(in_data, datap, ccd_sqsize);
	end_repack = timeGetTime();

	if(0)
	{
		dump_it(datap, (ccd_sqsize * n_ctrl) / (2 * det_bin), (ccd_sqsize * n_ctrl) / (2 * det_bin), 
					"file_raw", debug_img_no);
	    	debug_img_no++;
	}
	return(0);
  }

/*
 *	Handles the reception of corrected images to this gather process.
 *
 *	Changes for multiple frame grabber PCs.  We will be receiving data from multiple
 *	sockets containing the data.  Each frame grabber PC will send a header and its
 *	data.  The header from each frame grabber PC is identical.
 */

int	receive_data_cor(int ind)
  {
	char		*tstr;
	int		size_of_quadrant;
	char    	field[132];
	int		read_fds[MAX_CONTROLLERS];
	int		n,n_fds;
	double		atof();

	for(n = 0; n < n_ctrl; n++)
	    	read_fds[n] = data_fd[n];
	n_fds = n_ctrl;

	tstr = ztime();
	fprintf(stdout,"image_gather:  WAIT          at %s: image_type:   %s\n",tstr, "corrected");
	fflush(stdout);
	if(fpg != NULL)
	{
		fprintf(fpg,"image_gather:  WAIT          at %s: image_type:   %s\n",tstr, "corrected");
		fflush(fpg);
	}
	begin_recv = timeGetTime();
	  
	/*
	 *	If the data comes from a file, it will be multiples of ccd_sqsize.
	 *
	 *	If the data comes from the detector api, it will be a multiple of rows * cols.
	 */

	size_of_quadrant = (ccd_imsize * ccd_imsize * sizeof (unsigned short)) / (det_bin * det_bin);

	rep_read_mult_var_header(read_fds, n_fds, raw_header[1], in_data, &raw_header_size[1], size_of_quadrant);

	gethd("TIME", field, raw_header[ind - KIND_RAW_0]);
	found_time = atof(field);
	  
	time(&time_rec_start);
	tstr = ztime();
	end_recv = timeGetTime();
	fprintf(stdout,"image_gather:  RCVD          at %s: received in: %6.3f secs.\n",tstr, 
			(end_recv - begin_recv) / 1000.); 
	fflush(stdout);

	if(0)
	{
		dump_it(in_data, (ccd_imsize * n_ctrl) / (2 * det_bin), (ccd_imsize * n_ctrl) / (2 * det_bin), 
					"file_imin", debug_img_no);
	    	debug_img_no++;
	}

	begin_repack = timeGetTime();
	repack_memory(in_data, raw_data[ind - KIND_RAW_0], ccd_imsize);
	end_repack = timeGetTime();

	return(0);
  }

void	merge_headers(char *det_header, char *extra_header, char *merged_header)
{
	int	det_hlen;
	int	i, j, n;
	char	field[256], value[512], value2[512];

	gethdl(&det_hlen, det_header);
	for(i = 0; i < det_hlen; i++)
		merged_header[i] = det_header[i];
	for(n = 0; ; n++)
	{
		j = gethdn (n, field, value, extra_header);
		if(j == 0)
			break;
		value2[0] = '\0';
		gethd(field, value2, det_header);
		if(value2[0] == '\0')
		{
			if(0 == strcmp("SCAN_WAVELENGTH", field))
				puthd("WAVELENGTH", value, merged_header);		/* for adxv */
			puthd(field, value, merged_header);
		}
	}
	padhd(merged_header, 512);
}

int	is_smv_header(char *header)
{
	if(NULL != strstr(header, "{\nHEADER_BYTES="))
		return(1);

	return(0);
}

static char	logname_found[512];

static	int	use_malloc_header = 0;
static	char	redundant_header[CCD_HEADER_MAX];

void	strip_redundant_entries(char *hp)
{
	int	hdlen;
	int	i, last_size1, last_size2, last_saturated;
	char	*new_hd;
	char	field[256], value[512];
	int	ret, n_header_entries;

	logname_found[0] = '\0';
	gethdl(&hdlen, hp);
	if(0)
		fprintf(stdout,"strip_redundant_entries: hdlen: %d\n", hdlen);
	n_header_entries = 0;
	for(i = 0; ; i++)
	{
		if(';' == hp[i])
			n_header_entries++;
		if(0)
			fprintf(stdout,"%c", hp[i]);
		if('}' == hp[i])
			break;
	}
	if(0)
	{
	fprintf(stdout,"\nExact length of header was: %3d\n", i);
	fprintf(stdout,"\nNumber of header entries: %3d\n", n_header_entries);
	fflush(stdout);
	}
	if(use_malloc_header)
	{
		if(NULL == (new_hd = (char *) malloc(hdlen + 512)))
		{
			fprintf(stderr,"strip_redundant_entries: Error allocating %d bytes for scratch header\n",
						hdlen);
			return;
		}
	}
	else
		new_hd = &redundant_header[0];

	clrhd(redundant_header);
	last_size1 = 0;
	last_size2 = 0;
	last_saturated = 0;

	for(i = 0; i < n_header_entries; i++)
	{
		ret = gethdn ( i, field, value, hp );
		if(0 == ret)
			break;

		if(0)
		{
			fprintf(stdout,"strip_redundant_values: item: %4d ret: %d field: %s value: %s\n", i, ret, field, value);
			fflush(stdout);
		}

		if(0 == strcmp(field, "HEADER_BYTES"))
		{
			continue;
		}
		if(0 == strcmp(field, "SIZE1"))
		{
			last_size1 = atoi(value);
			continue;
		}
		if(0 == strcmp(field, "SIZE2"))
		{
			last_size2 = atoi(value);
			continue;
		}
		if(0 == strcmp(field, "CCD_IMAGE_SATURATION"))
		{
			last_saturated = atoi(value);
			continue;
		}
		if(0 == strcmp(field, "LOGNAME"))
		{
			strcpy(logname_found, value);
			if(NULL != (char *) getenv("CCD_NO_OUTPUT_LOGNAME"))
				continue;
		}
		puthd(field, value, redundant_header);
	}

	sprintf(value, "%d", last_size1);
	puthd("SIZE1", value, redundant_header);
	sprintf(value, "%d", last_size2);
	puthd("SIZE2", value, redundant_header);
	if(0 != last_saturated)
	{
		sprintf(value, "%d", last_saturated);
		puthd("CCD_IMAGE_SATURATION", value, redundant_header);
	}

	padhd(redundant_header, 512);
	gethdl(&hdlen, redundant_header);
	if(0)
		fprintf(stderr, "New header length after strip: %d\n", hdlen);
	for(i = 0; i < hdlen; i++)
		hp[i] = redundant_header[i];

	if(use_malloc_header)
		free(new_hd);
}	
	
/*
 *	Function to save a buffer to disk.
 *
 *	save_image(which_kind, buffer_kind)
 *
 *	We always start with the filename in "outfilename".
 *	An extension gets added to it based on the kind of buffer
 *	being saved.  One convention would have the dark current
 *	images end in .dkc and the regular images be .img.
 *
 *	Saved files would then be:
 *
 *		*.dkx_0		First dark current.
 *		*.dkx_1		Second dark current.
 *		*.dkx_2		Third dark current.
 *		*.dkx_3		Fourth dark current.
 *		*.dkc		Dezingered dark current.
 *		*.dkd		Dezingered dark current, second one if dzratio != 1.
 *		*.imx_0		First dark current.
 *		*.imx_1		First dark current.
 *		*.img		Dezingered (if reqested) & transformed.
 *
 *	buffer_kind is 0 for a direct copy of the raw file, 1 for
 *	the dezingered or transformed output version.
 */

int	save_image(int which_kind,int  buffer_kind, int compress)
  {
	char		actfname[1024],convbuf[20],compress_buf[1024];
	char		sizebuf[10];
	char		s[80];
	FILE		*fp;
	char		*cptr;
	unsigned short	*uptr;
	int		sizehd, sizedata, final_image_size, i, j;
	int		new_size1, new_size2;
	char		*tstr;
	float		*flptr;
	struct passwd	*pwp;
	char		*lit_size1 = "SIZE1";
	char		*lit_size2 = "SIZE2";

	cptr = NULL;
	uptr = NULL;
	strcpy(actfname,outfilename);
	if(0)
	  {
	    fprintf(stderr,"save_image: DEBUG: %s NOT WRITTEN\n",outfilename);
	    return(0);
	  }

	if(buffer_kind == 0)
	  {
	    if(rawfilename[0] == '\0')
	      {
	        if(which_kind % 2)
		    strcat(actfname,"_1");
	          else
		    strcat(actfname,"_0");
	      }
	     else
	      strcpy(actfname,rawfilename);
	  }
	time(&time_xform_end);
	tstr = ztime();

	if(NULL != (char *) strstr(actfname,"_null_"))
	  {
	    fprintf(stdout,"ccd_image_gather: by special convention (_null_), file %s is NOT written.\n",actfname);
	    fflush(stdout);
	    return(0);
	  }

	if(which_kind == KIND_DARK_3 && buffer_kind == 1)
		actfname[strlen(actfname) - 1] = 'd';

	if(NULL == (fp = fopen(actfname,"w")))
	  {
	    fprintf(stderr,"save_image: cannot create %s as output image\n",actfname);
	    return(1);
	  }

	/*
	 *	The size of raw file is always ccd_sqsize or ccd_sqsize * 2.
	 */

	final_image_size = ccd_sqsize / det_bin;
	if(n_ctrl == 4)
		 final_image_size *= 2;
	else if(n_ctrl == 9)
		 final_image_size *= 3;

	switch(which_kind)
	  {
	    case KIND_DARK_0:
		cptr = dkc_header[0];
		uptr = dkc_data[0];
		sizehd = dkc_header_size[0];
		sizedata = dkc_data_size[0];
		break;
	    case KIND_DARK_1:
		cptr = dkc_header[1];
		uptr = dkc_data[1];
		sizehd = dkc_header_size[1];
		sizedata = dkc_data_size[1];
		break;
	    case KIND_DARK_2:
		cptr = dkc_header[2];
		uptr = dkc_data[2];
		sizehd = dkc_header_size[2];
		sizedata = dkc_data_size[2];
		break;
	    case KIND_DARK_3:
		cptr = dkc_header[3];
		uptr = dkc_data[3];
		sizehd = dkc_header_size[3];
		sizedata = dkc_data_size[3];
		break;
	    case KIND_RAW_0:
		cptr = raw_header[0];
		uptr = raw_data[0];
		sizehd = raw_header_size[0];
		sizedata = raw_data_size[0];
		break;
	    case KIND_RAW_1:
		if(buffer_kind == 0)
		  {
		    cptr = raw_header[1];
		    uptr = raw_data[1];
		    sizehd = raw_header_size[1];
		    sizedata = raw_data_size[1];
		  }
		 else
		  {
		    /*
		     *	Watch this one: very kludgey
		     *
		     *	Data is in 0, but header source is 1.  Yeech.
		     */

		    cptr = raw_header[1];
		    uptr = raw_data[0];
		    sizehd = raw_header_size[1];
		    sizedata = raw_data_size[1];
		  }
		break;
	  }

	if(merge_header_bytes > 0 && is_smv_header(merge_header))
	{
		fprintf(stderr,"calling merge_headers\n");
		merge_header[merge_header_bytes] = '\0';
		merge_headers(cptr, merge_header, input_header);
		cptr = &input_header[0];
	}

	sizedata = (ccd_sqsize * ccd_sqsize * n_ctrl) / (det_bin * det_bin);

	sprintf(sizebuf, "%d", final_image_size);
	puthd(lit_size1, sizebuf, cptr);
	sprintf(sizebuf, "%d", final_image_size);
	puthd(lit_size2, sizebuf, cptr);

	padhd(cptr,512);

	(void) gethdl(&sizehd,cptr);

/*
 *	Check for consistency in byte ordering.  Needs to be fixed if
 *	the byte order of ccd_dc_api is different from that of the
 *	transform.
 */

	gethd("BYTE_ORDER",s,cptr);
	if(0)
	{
	if(getbo() == 0 && 0 == strcmp(s,"big_endian"))
	  {
	    puthd("BYTE_ORDER","little_endian",cptr);
	    padhd(cptr,512);
	    (void) gethdl(&sizehd,cptr);
	  }
	 else
	  if(getbo() == 1 && 0 == strcmp(s,"little_endian"))
	    {
	    puthd("BYTE_ORDER","big_endian",cptr);
	    padhd(cptr,512);
	    (void) gethdl(&sizehd,cptr);
	    }
	}

	if((req_ave == 0) && (NULL != (char *) strstr(actfname,".dkc")))
	  {
	        sprintf(convbuf,"%.2f",found_time * 2);
	        puthd("TIME",convbuf,cptr);
	        padhd(cptr,512);
	        (void) gethdl(&sizehd,cptr);
	  }

        if(use_strips)
          {
	    flptr = sav;
	    if(NULL != (char *) strstr(actfname,".dkc"))
		flptr = sav_c;
	      else
		if(NULL != (char *) strstr(actfname,".dkd"))
		  flptr = sav_d;
	    if(0)	/* DEBUG UNTIL THE MOSFLM BUG OF > 512 BYTE HEADERS FIXED */
	    {
            if(n_ctrl == 1)
              {
                sprintf(convbuf,"%.1f",flptr[0]);
                puthd("PED_A",convbuf,cptr);
              }
             else
              {
                sprintf(convbuf,"%.1f",flptr[0]);
                puthd("PED_A",convbuf,cptr);
                sprintf(convbuf,"%.1f",flptr[1]);
                puthd("PED_B",convbuf,cptr);
                sprintf(convbuf,"%.1f",flptr[2]);
                puthd("PED_C",convbuf,cptr);
                sprintf(convbuf,"%.1f",flptr[3]);
                puthd("PED_D",convbuf,cptr);
              }
	    }
            padhd(cptr,512);
            (void) gethdl(&sizehd,cptr);
          }
	if(1)
	{
		s[0] = '\0';
		gethd("CCD",s,cptr);
		if('\0' != s[0])
		{
			if(0 == strcmp(s, "PAD"))
			{
				gethd("SIZE1", s, cptr);
				new_size1 = atoi(s) / 2;
				gethd("SIZE2", s, cptr);
				new_size2 = atoi(s) / 2;
				puthd("TYPE", "signed_integer", cptr);
				sprintf(convbuf, "%d", new_size1);
				puthd("SIZE1", convbuf, cptr);
				sprintf(convbuf, "%d", new_size2);
				puthd("SIZE2", convbuf, cptr);
			}
		}
	}

	padhd(cptr, 512);
	(void) gethdl(&sizehd, cptr);

	strip_redundant_entries(cptr);
	gethdl(&sizehd, cptr);

	if(sizehd != fwrite(cptr,sizeof (char), sizehd, fp))
	  {
	    fprintf(stderr,"save_image: error writing header of %s\n",actfname);
	    fclose(fp);
	    return(1);
	  }

	if(sizedata != fwrite(uptr,sizeof (unsigned short), sizedata, fp))
	  {
	    fprintf(stderr,"save_image: error writing data of %s\n",actfname);
	    fclose(fp);
	    return(1);
	  }

	fclose(fp);

	if(logname_found[0] != '\0')
	{
		if(0 != (pwp = getpwnam(logname_found)))
		{
			chown(actfname, pwp->pw_uid, pwp->pw_gid);
		}
	}

	if(compress && outfile_type == OUTFILE_IMG_ONLY)
	  {
	    sprintf(compress_buf,"compress %s",actfname);
	    system(compress_buf);
	  }
	if(buffer_kind == 1 && which_kind == KIND_RAW_1)
	  {
	    xform_counter++;
	    if(NULL != (fpxfs = fopen(xfsname,"w")))
	      {
	        fprintf(fpxfs,"%d %s\n",xform_counter,actfname);
	        fclose(fpxfs);
	      }
	    send_to_adxv_socket(actfname);
	  }

	tstr = ztime();
	end_disk_write = timeGetTime();
	
	fprintf(stdout,"image_gather:  RPAK          at %s: repack  in:  %6.3f secs.\n",tstr, 
			(end_repack - begin_repack) / 1000.); 

	fprintf(stdout,"image_gather:  TODK          at %s: to disk in:  %6.3f secs.\n",tstr, 
			(end_disk_write - end_repack) / 1000.); 
	if(fpg != NULL)
	{
		fprintf(fpg,"image_gather:  TODK          at %s: to disk in:  %6.3f secs.\n",tstr, 
			(end_disk_write - end_recv) / 1000.); 
		fflush(fpg);
	}
	j = strlen(actfname);
	cptr = actfname;
	for(i = j - 1; i > 0; i--)
		if(actfname[i] == '/')
		{
			cptr = &actfname[i + 1];
			break;
		}
	fprintf(stdout,"image_gather:  DONE          at %s: wrote:        %s\n",tstr, 
			cptr);
	fflush(stdout);
	if(fpg != NULL)
	{
		fprintf(fpg,"image_gather:  DONE          at %s: wrote:        %s\n",tstr, 
			actfname);
		fflush(fpg);
	}

	return(0);
  }

int	save_image_with_convert_smv(int compress)
  {
	register unsigned short	*uptr;
	char		actfname[1024],convbuf[20],compress_buf[1024];
	char		sizebuf[10];
	FILE		*fp;
	char		*hdptr;
	char		*cptr;
	int		sizehd,sizedata;
	char		*tstr;
	int		i,j;
	int		new_size1, new_size2;
	int		out_x, out_y;
	int		final_image_size;
	char		s[80];
	struct passwd	*pwp;
	int		img2jpg_sub(unsigned short *raw, int size_x, int size_y, int out_x, int out_y, char *outfile);

	strcpy(actfname,outfilename);

	if(0)
	  {
	    fprintf(stderr,"save_image: DEBUG: %s NOT WRITTEN\n",outfilename);
	    return(0);
	  }

	time(&time_xform_end);

	if(NULL != (char *) strstr(actfname,"_null_"))
	  {
	    fprintf(stdout,"ccd_image_gather: by special convention (_null_), file %s is NOT written.\n",actfname);
	    fflush(stdout);
	    return(0);
	  }

	if(NULL == (fp = fopen(actfname,"w")))
	  {
	    fprintf(stderr,"ccd_image_gather: save_image: cannot create %s as output image\n",actfname);
	    return(1);
	  }

	/*
	 *	The size of the data is (ccd_imsize * ccd_imsize * n_ctrl) / (det_bin * det_bin)
	 *
	 *	SIZE1 and SIZE2 are always ccd_imsize / det_bin  or (ccd_imsize * 2) / det_bin [n_ctrl == 4].
	 */

	sizedata = (ccd_imsize * ccd_imsize * n_ctrl) / (det_bin * det_bin);
	i = ccd_imsize / det_bin;
	if(n_ctrl == 4)
		i *= 2;
	else if(n_ctrl == 9)
		i *= 3;
	sprintf(sizebuf,"%d",i);
	final_image_size = i;
	if(did_dezingering)
	  {
		sizehd = raw_header_size[0];
		hdptr = raw_header[0];
	  }
	 else
	  {
		sizehd = raw_header_size[1];
		hdptr = raw_header[1];
	  }
	puthd("SIZE1",sizebuf,hdptr);
	puthd("SIZE2",sizebuf,hdptr);

/*
 *	Check for consistency in byte ordering.  Needs to be fixed if
 *	the byte order of ccd_dc_api is different from that of the
 *	transform.
 */

	if(0)
	{
	gethd("BYTE_ORDER",s,hdptr);
	if(getbo() == 0 && 0 == strcmp(s,"big_endian"))
	  {
	    puthd("BYTE_ORDER","little_endian",hdptr);
	    padhd(hdptr,512);
	    (void) gethdl(&sizehd,hdptr);
	  }
	 else
	  if(getbo() == 1 && 0 == strcmp(s,"little_endian"))
	    {
	    puthd("BYTE_ORDER","big_endian",hdptr);
	    padhd(hdptr,512);
	    (void) gethdl(&sizehd,hdptr);
	    }
	}
	if(1)
	{
		s[0] = '\0';
		gethd("CCD",s,hdptr);
		if('\0' != s[0])
		{
			if(0 == strcmp(s, "PAD"))
			{
				gethd("SIZE1", s, hdptr);
				new_size1 = atoi(s) / 2;
				gethd("SIZE2", s, hdptr);
				new_size2 = atoi(s) / 2;
				puthd("TYPE", "signed_integer", hdptr);
				sprintf(convbuf, "%d", new_size1);
				puthd("SIZE1", convbuf, hdptr);
				sprintf(convbuf, "%d", new_size2);
				puthd("SIZE2", convbuf, hdptr);
			}
		}
	}

	if((req_ave == 0) && (did_dezingering == 1))
	  {
	        sprintf(convbuf,"%.2f",found_time * 2);
	        puthd("TIME",convbuf,hdptr);
	  }

	if(merge_header_bytes > 0 && is_smv_header(merge_header))
	{
		merge_header[merge_header_bytes] = '\0';
		merge_headers(hdptr, merge_header, input_header);
		hdptr = &input_header[0];
	}

	padhd(hdptr,512);
	(void) gethdl(&sizehd,hdptr);

	strip_redundant_entries(hdptr);

	(void) gethdl(&sizehd,hdptr);	/* possible for the header to shrink after strip_redundant_entries */

	if(sizehd != fwrite(hdptr,sizeof (char), sizehd, fp))
	  {
	    fprintf(stderr,"ccd_image_gather: save_image: error writing header of %s\n",actfname);
	    fclose(fp);
	    return(1);
	  }

	if(1)
	{
	    uptr = (unsigned short *) raw_data[0];

	    if(sizedata != fwrite(uptr,sizeof (unsigned short), sizedata, fp))
	      {
	        fprintf(stderr,"ccd_image_gather: save_image: error writing data of %s\n",actfname);
	        fclose(fp);
	        return(1);
	      }
	}

	fclose(fp);

	if(logname_found[0] != '\0')
	{
		if(0 != (pwp = getpwnam(logname_found)))
		{
			chown(actfname, pwp->pw_uid, pwp->pw_gid);
		}
	}

	if(compress)
	  {
	    sprintf(compress_buf,"compress %s",actfname);
	    system(compress_buf);
	  }

	if(jpeg1_name[0] != '\0')
	{
		sscanf(jpeg1_size, "%dx%d", &out_x, &out_y);
		img2jpg_sub(uptr, final_image_size, final_image_size, out_x, out_y, jpeg1_name);
		tstr = ztime();
		fprintf(stdout,"image_gather:  JPG1          at %s: wrote:        %s  (%s)\n",tstr, jpeg1_name, jpeg1_size);
		jpeg1_name[0] = '\0';
	}
	if(jpeg2_name[0] != '\0')
	{
		sscanf(jpeg2_size, "%dx%d", &out_x, &out_y);
		img2jpg_sub(uptr, final_image_size, final_image_size, out_x, out_y, jpeg2_name);
		tstr = ztime();
		fprintf(stdout,"image_gather:  JPG2          at %s: wrote:        %s  (%s)\n",tstr, jpeg2_name, jpeg2_size);
		jpeg2_name[0] = '\0';
	}

	xform_counter++;
	if(NULL != (fpxfs = fopen(xfsname,"w")))
	  {
	    fprintf(fpxfs,"%d %s\n",xform_counter,actfname);
	    fclose(fpxfs);
	  }
	send_to_adxv_socket(actfname);
	tstr = ztime();
	end_disk_write = timeGetTime();
	fprintf(stdout,"image_gather:  RPAK          at %s: repack  in:  %6.3f secs.\n",tstr, 
			(end_repack - begin_repack) / 1000.); 

	fprintf(stdout,"image_gather:  TODK          at %s: to disk in:  %6.3f secs.\n",tstr, 
			(end_disk_write - end_repack) / 1000.); 
	j = strlen(actfname);
	cptr = actfname;
	for(i = j - 1; i > 0; i--)
		if(actfname[i] == '/')
		{
			cptr = &actfname[i + 1];
			break;
		}
	fprintf(stdout,"image_gather:  DONE          at %s: wrote:        %s\n",tstr, 
			cptr);
	fflush(stdout);
	return(0);
  }

/*
 *	save image using cbf format.
 *
 *	Convert the header from SMV to CBF.
 *
 *	The compress argument is for the time being ignored (always compress).
 */

int	save_image_with_convert_cbf(int compress)
  {
	register unsigned short	*uptr;
	char		actfname[1024],convbuf[20];
	char		sizebuf[10];
	char		*cptr;
	char		*hdptr;
	int		sizehd,sizedata;
	char		*tstr;
	int		i,j;
	struct passwd	*pwp;

#ifdef USE_CBF

	int     	adscimg2cbf_sub(char *header, unsigned short *data, char *cbf_filename, int pack_flags);
#define CBF_BYTE_OFFSET 0x0070  	/* Byte Offset Compression            */
	int		pack_flags = CBF_BYTE_OFFSET;
	char		*cp;

	if(NULL != (cp = getenv("CBF_PACK_FLAGS")))
		sscanf(cp, "%x", &pack_flags);

#endif /* USE_CBF */

	strcpy(actfname,outfilename);

	time(&time_xform_end);

	if(NULL != (char *) strstr(actfname,"_null_"))
	{
	    fprintf(stdout,"ccd_image_gather: by special convention (_null_), file %s is NOT written.\n",actfname);
	    fflush(stdout);
	    return(0);
	}

	/*
	 *	The size of the data is (ccd_imsize * ccd_imsize * n_ctrl) / (det_bin * det_bin)
	 *
	 *	SIZE1 and SIZE2 are always ccd_imsize / det_bin  or (ccd_imsize * 2) / det_bin [n_ctrl == 4].
	 */

	sizedata = (ccd_imsize * ccd_imsize * n_ctrl) / (det_bin * det_bin);
	i = ccd_imsize / det_bin;
	if(n_ctrl == 4)
		i *= 2;
	else if(n_ctrl == 9)
		i *= 3;
	sprintf(sizebuf,"%d",i);
	if(did_dezingering)
	{
		sizehd = raw_header_size[0];
		hdptr = raw_header[0];
	}
	else
	{
		sizehd = raw_header_size[1];
		hdptr = raw_header[1];
	}
	puthd("SIZE1",sizebuf,hdptr);
	puthd("SIZE2",sizebuf,hdptr);

	if((req_ave == 0) && (did_dezingering == 1))
	{
	        sprintf(convbuf,"%.2f",found_time * 2);
	        puthd("TIME",convbuf,hdptr);
	}

	if(merge_header_bytes > 0 && is_smv_header(merge_header))
	{
		merge_header[merge_header_bytes] = '\0';
		merge_headers(hdptr, merge_header, input_header);
		hdptr = &input_header[0];
		merge_header[0] = '\0';
	}

	padhd(hdptr,512);
	(void) gethdl(&sizehd,hdptr);

	strip_redundant_entries(hdptr);

	(void) gethdl(&sizehd,hdptr);	/* possible for the header to shrink after strip_redundant_entries */

	uptr = (unsigned short *) raw_data[0];

	if(NULL != (cptr = (char *) strstr(actfname, ".img")))
		sprintf(cptr, "%s", ".cbf");

#ifdef USE_CBF
	if(adscimg2cbf_sub (hdptr, uptr, actfname, pack_flags))
	{
		fprintf(stderr,"ccd_image_gather: save_image: error generating CBF file: %s\n",actfname);
		return(1);
	}
#endif /* USE_CBF */

	if(logname_found[0] != '\0')
	{
		if(0 != (pwp = getpwnam(logname_found)))
		{
			chown(actfname, pwp->pw_uid, pwp->pw_gid);
		}
	}

	xform_counter++;
	if(NULL != (fpxfs = fopen(xfsname,"w")))
	  {
	    fprintf(fpxfs,"%d %s\n",xform_counter,actfname);
	    fclose(fpxfs);
	  }
	send_to_adxv_socket(actfname);
	tstr = ztime();
	end_disk_write = timeGetTime();
	fprintf(stdout,"image_gather:  RPAK          at %s: repack  in:  %6.3f secs.\n",tstr, 
			(end_repack - begin_repack) / 1000.); 

	fprintf(stdout,"image_gather:  TODK          at %s: to disk in:  %6.3f secs.\n",tstr, 
			(end_disk_write - end_repack) / 1000.); 
	j = strlen(actfname);
	cptr = actfname;
	for(i = j - 1; i > 0; i--)
		if(actfname[i] == '/')
		{
			cptr = &actfname[i + 1];
			break;
		}
	fprintf(stdout,"image_gather:  DONE          at %s: wrote:        %s\n",tstr, 
			cptr);
	fflush(stdout);
	return(0);
  }
