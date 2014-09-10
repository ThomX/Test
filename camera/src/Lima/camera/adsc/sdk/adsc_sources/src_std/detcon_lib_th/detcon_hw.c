#include	"detcon_ext.h"

/*
 *----------------------------------------------
 *
 *	Module to sequence the control of the CCD
 *	hardware modules.
 *
 *	Two processes are used to run hardware:
 *		det_api_workstation	command interface to the frame grabber computers
 *		ccd_image_gather	receives data from the frame grabber computers.
 *
 *----------------------------------------------
 */

#define KIND_DARK_0     0
#define KIND_DARK_1     1
#define KIND_DARK_2     2
#define KIND_DARK_3     3
#define KIND_RAW_0      4
#define KIND_RAW_1      5

void	strcat_between(char *to, char *from, char *between_this, char *and_that)
{
	char	*cp1, *cp2, *cp_from, *cp_to;

	if(NULL == (cp1 = strstr(from, between_this)))
		return;
	if(NULL == (cp2 = strstr(from, and_that)))
		return;

	cp1 += strlen(between_this);

	cp_to = to + strlen(to);

	for(cp_from = cp1; cp_from < cp2; cp_from++)
		*cp_to++ = *cp_from;
	*cp_to = '\0';
}

void	detcon_sleep(int nmsec)
{
	usleep(1000 * nmsec);
}

/*
 *	Take a list of standard header items (below) and allow a buffer
 *	with value1=key1\n...valuen=keyn\n\0 to reassign the values from
 *	this buffer before the final header is constructed in detcon_make_header_smv
 *
 *	This is a courtesy for the lima plug-in.
 */

struct redi {
		char	*name;
		int	fpar;
	    };

struct redi	redefinable_items_float[]  = {
					{"OMEGA",	FLP_OMEGA},
					{"KAPPA",	FLP_KAPPA},
					{"PHI", 	FLP_PHI},
					{"TWOTHETA",	FLP_TWOTHETA},
					{"DISTANCE",	FLP_DISTANCE},
					{"WAVELENGTH",	FLP_WAVELENGTH},
					{"OSC_RANGE",	FLP_OSC_RANGE},
					{"TIME",	FLP_TIME},
					{"DOSE",	FLP_DOSE},
					{"BEAM_CENTER_X",FLP_BEAM_X},
					{"BEAM_CENTER_Y",FLP_BEAM_Y},
					{NULL,		0}
				      };

struct redi	redefinable_items_int[]  = {
					{"AXIS",	FLP_AXIS},
					{NULL,		0}
				    };

int	detcon_redefine_headerparams(char *buf)
{
	int	i;
	char	*cp, *cpe;
	float	fval;
	int	ival;

	//  fprintf(stdout, "detcon_redefine_headerparams:\n%s\n", buf);
	for(i = 0; redefinable_items_float[i].name != NULL; i++)
	{
		if(NULL != (cp = strstr(buf, redefinable_items_float[i].name)))
		{
			if(NULL != (cpe = strstr(cp, "=")))
			{
				fval = atof(cpe + 1);
				//  fprintf(stdout, "Setting parameter %s (number %d) to %.6f\n",
				//	redefinable_items_float[i].name, redefinable_items_float[i].fpar, fval);
				CCDSetFilePar(redefinable_items_float[i].fpar, (char *) &fval);
				continue;
			}
		}
	}

	for(i = 0; redefinable_items_int[i].name != NULL; i++)
	{
		if(NULL != (cp = strstr(buf, redefinable_items_int[i].name)))
		{
			if(NULL != (cpe = strstr(cp, "=")))
			{
				ival = atoi(cpe + 1);
				CCDSetFilePar(redefinable_items_int[i].fpar, (char *) &ival);
				continue;
			}
		}
	}
	return(0);
}

/*
 *	detcon_make_header_smv  -  make a suitable SMV header.
 */

#define	SHDSIZE		20480

static	char	made_header[SHDSIZE];

static	char	file_part1[1024];
static	char	file_part2[1024];
static	char	file_suffix[20];
static	char	file_seq[1024];

void	detcon_make_header_smv()
{
	char	buf[32];
	char	*cptr;
	int	endian_test;

	detcon_redefine_headerparams(dtc_headerparams);

	clrhd(made_header);

	/*
	 *	standard items.
	 */
	
	puthd("DIM","2",made_header);

	endian_test = 1;
	if((*(char *)&endian_test) == 1)
	    puthd("BYTE_ORDER","little_endian",made_header);
	else
	    puthd("BYTE_ORDER","big_endian",made_header);

	puthd("TYPE","unsigned_short",made_header);

	if(dtc_stat_bin == 1)
	  {
	    sprintf(buf,"%d",dtc_chip_size_x);
	    puthd("SIZE1",buf,made_header);
	    sprintf(buf,"%d",dtc_chip_size_y);
	    puthd("SIZE2",buf,made_header);
	    sprintf(buf,"%.6f",dtc_pixel_size);
	    puthd("PIXEL_SIZE",buf,made_header);
	    puthd("BIN","none",made_header);
	  }
	 else
	  {
	    sprintf(buf,"%d",dtc_chip_size_x / 2);
	    puthd("SIZE1",buf,made_header);
	    sprintf(buf,"%d",dtc_chip_size_y / 2);
	    puthd("SIZE2",buf,made_header);
	    sprintf(buf,"%.6f",dtc_pixel_size * 2);
	    puthd("PIXEL_SIZE",buf,made_header);
	    puthd("BIN","2x2",made_header);
	  }
	if(dtc_stat_adc == 0)
	    puthd("ADC","slow",made_header);
	  else
	    puthd("ADC","fast",made_header);
	/*
	 *	adsc items.
	 */
	if(dtc_detector_sn > 0)
	  {
		sprintf(buf,"%d",dtc_detector_sn);
		puthd("DETECTOR_SN",buf,made_header);
	  }
	cptr = dtc_ztime();
	puthd("DATE",cptr,made_header);
	strcpy(file_seq, cptr);
	sprintf(buf,"%.6f",dtc_stat_time);
	puthd("TIME",buf,made_header);
	sprintf(buf,"%.3f",dtc_stat_dist);
	puthd("DISTANCE",buf,made_header);
	sprintf(buf,"%.3f",dtc_stat_osc_width);
	puthd("OSC_RANGE",buf,made_header);
	if(dtc_stat_axis == 1)
	  {
	    if(dtc_wasset_kappa)
	      {
		sprintf(buf,"%.3f",dtc_stat_phi);
		puthd("PHI",buf,made_header);
		puthd("OSC_START",buf,made_header);
		sprintf(buf,"%.3f",dtc_stat_omega);
		puthd("OMEGA",buf,made_header);
	        sprintf(buf,"%.3f",dtc_stat_kappa);
	        puthd("KAPPA",buf,made_header);
	      }
	     else
	      {
		sprintf(buf,"%.3f",dtc_stat_phi);
		puthd("PHI",buf,made_header);
		puthd("OSC_START",buf,made_header);
	      }
	  }
	 else		/* if axis = 0, omega setting rules.... */
	  {
	    if(dtc_wasset_kappa)
	      {
		sprintf(buf,"%.3f",dtc_stat_omega);
		puthd("OMEGA",buf,made_header);
		puthd("OSC_START",buf,made_header);
		sprintf(buf,"%.3f",dtc_stat_phi);
		puthd("PHI",buf,made_header);
	        sprintf(buf,"%.3f",dtc_stat_kappa);
	        puthd("KAPPA",buf,made_header);
	      }
	     else
	      {
		sprintf(buf,"%.3f",dtc_stat_omega);
		puthd("OMEGA",buf,made_header);
		puthd("OSC_START",buf,made_header);
	      }
	  }
	if(dtc_wasset_2theta)
	  {
	    sprintf(buf,"%.3f",dtc_stat_2theta);
	    puthd("TWOTHETA",buf,made_header);
	  }
	if(dtc_stat_axis == 1)
	    puthd("AXIS","phi",made_header);
	  else
	    puthd("AXIS","omega",made_header);
	if(dtc_wasset_wavelength)
	{
		sprintf(buf,"%.4f",dtc_stat_wavelength);
		puthd("WAVELENGTH",buf,made_header);
	}
	sprintf(buf,"%.3f",dtc_stat_xcen);
	puthd("BEAM_CENTER_X",buf,made_header);
	sprintf(buf,"%.3f",dtc_stat_ycen);
	puthd("BEAM_CENTER_Y",buf,made_header);
	padhd(made_header,512);
}

/*
 *	Routine to send the start detector exposing command.  Main purpose
 *	here is to build up filenames.
 */

void	*detcon_send_det_start(void *arg)
{
	char	tempbuf[2048],tbuf[2048],infobuf[1024];
	int	hsize;
        int  	detret;
	int	i,dark;


	log_info("detcon_send_det_start","started",0);

        while(-1 == dtc_fddetcmd)
        {
		if(DTC_STATE_CONFIGDET != dtc_state)
			setDTCState(DTC_STATE_CONFIGDET, "in detcon_send_det_start: waiting for det_api_workstation to connect", 0);
		detcon_sleep(1000);
        }

        while(-1 == dtc_xfdatafd)
        {
		if(DTC_STATE_CONFIGDET != dtc_state)
			setDTCState(DTC_STATE_CONFIGDET, "in detcon_send_det_start: waiting for data file descriptors to connect", 0);
            	detcon_output_detcmd(dtc_fddetcmd,"getparam\nxfdatafd\n",NULL,0);
        	if(NULL != (char *) strstr(dtc_det_reply,"OK"))
		{
                    if('0' == dtc_det_reply[3])
                    {
			detcon_sleep(1000);
			continue;
                    }
                    dtc_xfdatafd = 1;
		}
		else
		{
			detcon_sleep(1000);
			continue;
		}
	}

	i = strlen(dtc_filename);
	strcpy(file_part1,dtc_filename);
	if(i > 4 && 0 == strcmp(dtc_default_suffix,&dtc_filename[i - 4]))
		file_part1[i-4] = '\0';
	i = strlen(file_part1);
	for(; i > 0; i--)
	  if(file_part1[i - 1] == '/')
		break;
	strcpy(file_part2,&file_part1[i]);

	dark = 0;
	switch(dtc_image_kind)
	  {
	    case KIND_DARK_0:
		strcpy(file_suffix,".dkx_0");
		dark = 1;
		break;
	    case KIND_DARK_1:
		strcpy(file_suffix,".dkx_1");
		dark = 1;
		break;
	    case KIND_DARK_2:
		strcpy(file_suffix,".dkx_2");
		dark = 1;
		break;
	    case KIND_DARK_3:
		strcpy(file_suffix,".dkx_3");
		dark = 1;
		break;
	    case KIND_RAW_0:
		strcpy(file_suffix,".imx_0");
		dark = 0;
		break;
	    case KIND_RAW_1:
		strcpy(file_suffix,".imx_1");
		dark = 0;
		break;
	  }
	if(dark == 1)
		strcpy(dtc_status_string,"Taking Dark Image");
	  else
		strcpy(dtc_status_string,"Taking Exposure");

	detcon_make_header_smv();
	gethdl(&hsize, made_header);

	sprintf(tempbuf,"start\nheader_size %d\n",hsize);

	//sprintf(infobuf,"info %s%s\n",file_part2,file_suffix);
	sprintf(infobuf,"info %s%s\n", dtc_filename, file_suffix);

	sprintf(tbuf,"row_xfer %d\ncol_xfer %d\n",dtc_chip_size_y / dtc_stat_bin, dtc_chip_size_x / dtc_stat_bin);
	strcat(tempbuf, tbuf);

	if(dtc_use_j5_trigger)
	      strcat(tempbuf,"j5_trigger 1\n");

	strcat(tempbuf,infobuf);
	sprintf(tbuf,"adc %d\nrow_bin %d\ncol_bin %d\ntime %f\n",dtc_stat_adc,dtc_stat_bin,dtc_stat_bin,dtc_stat_time);
	strcat(tempbuf,tbuf);
	if(dtc_modular)	/* for multiprocessor version */
	{
		sprintf(tbuf,"transform_image %d\nsave_raw %d\nimage_kind %d\n",
			!dtc_no_transform, dtc_output_raws, dtc_image_kind);
		strcat(tempbuf, tbuf);
	}
	if(dtc_loadfile)
	{
		sprintf(tbuf,"loadfile %s\n", dtc_read_filename);
		strcat(tempbuf, tbuf);
	}
	if(dtc_stored_dark)
		strcat(tempbuf, "stored_dark 1\n");

	if(dtc_userdef_str[0] != '\0')
	{
		sprintf(tbuf,"userdef_str %s\n", dtc_userdef_str);
		strcat(tempbuf, tbuf);
	}
	/*
	 *	multi frame pad detector is "enabled" with dtc_mult_nframe > 0
	 */
	if(dtc_mult_nframe > 1)
	{
		sprintf(tbuf, "mult_frame_nframe %d\n", dtc_mult_nframe);
		strcat(tempbuf, tbuf);

		sprintf(tbuf, "mult_frame_trigtype %d\n", dtc_mult_trigtype);
		strcat(tempbuf, tbuf);
	}

	log_info("detcon_send_det_start calling detcon_output_detcmd_issue",tempbuf,0);

	pthread_mutex_lock(&dtc_h_mutex_fddetcmd);
	detret = detcon_output_detcmd_issue(dtc_fddetcmd,tempbuf,made_header,hsize);

        if (detret == CCD_DET_NOTCONNECTED) 
	{
		dtc_loadfile = 0;
		dtc_userdef_str[0] = '\0';
		setDTCState(DTC_STATE_ERROR, "in detcon_send_det_start: Detector process (det_api_workstation) NOT connected", 0);
		pthread_mutex_unlock(&dtc_h_mutex_fddetcmd);
		return(NULL);
        }

	detret = detcon_output_detcmd_receive(dtc_fddetcmd);

	if(detret != CCD_DET_OK)
	{
		setDTCState(DTC_STATE_ERROR, "in detcon_send_det_start: detcon_send_det_start_continue: Error returned",  0);
		pthread_mutex_unlock(&dtc_h_mutex_fddetcmd);
		return(NULL);
	}
	pthread_mutex_unlock(&dtc_h_mutex_fddetcmd);

	setDTCState(DTC_STATE_EXPOSING, "in detcon_send_det_start: Detector is exposing",  0);

	log_info("detcon_send_det_start", "return",0);
	return(NULL);
}



/*
 *	Routine to handle the checking of the detector read operation.
 */

void	*detcon_send_det_stop(void *arg)
{
	int	detret, loading_file;
	char	*dtc_ztime();

	/*
	 *	Issue stop, return immediately.
	 */

	loading_file = dtc_loadfile;	/* save for below */
	dtc_loadfile = 0;
	strcpy(dtc_status_string,"Reading Detector");

	pthread_mutex_lock(&dtc_h_mutex_fddetcmd);
	detret = detcon_output_detcmd_issue(dtc_fddetcmd,"stop\n",NULL,0);

	if(detret != CCD_DET_OK)
	{
		setDTCState(DTC_STATE_ERROR, "in detcon_send_det_stop: Detector process (det_api_workstation) disconnected during stop (stopr)", 0);
		pthread_mutex_unlock(&dtc_h_mutex_fddetcmd);
		return(NULL);
	}
	pthread_mutex_unlock(&dtc_h_mutex_fddetcmd);

	setDTCState(DTC_STATE_READING, "in detcon_send_det_stop: Detector is reading out (stopr begun)", 0);

	pthread_mutex_lock(&dtc_h_mutex_fddetcmd);
	detret = detcon_output_detcmd_receive(dtc_fddetcmd);
	if(detret != CCD_DET_OK)
	{
	  	if(detret != CCD_DET_RETRY)
		{
			fprintf(stdout,"detcon_send_det_continue: FATAL ERROR returned\n");
			setDTCState(DTC_STATE_ERROR, "in detcon_send_det_stop: ERROR (FATAL) returned from detector stop (stopw command)", 0);
			pthread_mutex_unlock(&dtc_h_mutex_fddetcmd);
			return(NULL);
		}
		else
		{
			fprintf(stdout,"detcon_send_det_continue: RETRYABLE error signaled.\n");
			setDTCState(DTC_STATE_RETRY, "in detcon_send_det_stop: ERROR (RETRYABLE) returned from detector stop (stopw command)", 0);
			pthread_mutex_unlock(&dtc_h_mutex_fddetcmd);
			return(NULL);
		}
	}
	if(dtc_autosample_temp_per_image)
	{
		detcon_get_temperature_readings_fd(dtc_fddetcmd);
	}

	pthread_mutex_unlock(&dtc_h_mutex_fddetcmd);

	if(dtc_userdef_str[0] != '\0')
	{
		pthread_mutex_lock(&dtc_h_mutex_fddetcmd);
		detret = detcon_output_detcmd_issue(dtc_fddetcmd,"getparam\nuserret\n",NULL,0);
		detret = detcon_output_detcmd_receive(dtc_fddetcmd);
		pthread_mutex_unlock(&dtc_h_mutex_fddetcmd);
	}
	setDTCState(DTC_STATE_IDLE, "in detcon_send_det_stop: return to IDLE after stop command finished", 0);

	strcpy(dtc_status_string,"Idle");

	return(NULL);
}

void	*detcon_check_gather_return(void *arg)
{
	int	ret;

	while(1)
	{
		detcon_sleep(1000);

		if(pthread_mutex_trylock(&dtc_h_mutex_fdxfcm))
			continue;
		if(-1 == dtc_fdxfcm)
		{
			pthread_mutex_unlock(&dtc_h_mutex_fdxfcm);
			continue;
		}

		if(0 == detcon_check_port_ready(dtc_fdxfcm))
		{
			pthread_mutex_unlock(&dtc_h_mutex_fdxfcm);
			continue;
		}

		ret = detcon_output_gather_receive(dtc_fdxfcm);

		if(ret != CCD_DET_OK)
		{
			log_info("detcon_check_gather_return: ERROR received", dtc_xform_reply, 0);
			dtc_received_gather_error = 1;
		}
		pthread_mutex_unlock(&dtc_h_mutex_fdxfcm);
	}
}

int	detcon_send_copy_command()
{
	int	len_xfcmd;
	char	xfcmd_buf[20480];
	char	suffix[20];
	char	body[2048];
	char	dzstuff[2048];
	int	hsize;
	int	do_reply;
	int	merge_header_bytes;
	int	xs, ys;

	if(dtc_check_gather_return)
	{
		do_reply = 1;
	}
	else
		do_reply = 0;

	gethdl(&hsize, made_header);

	if( dtc_mult_nframe <= 0)
	{
		xs = dtc_chip_size_x / dtc_stat_bin;
		ys = dtc_chip_size_x / dtc_stat_bin;
	}
	else	/* pad multi image; use the actual size of the array */
	{
		xs = dtc_chip_size_x / dtc_stat_bin;
		ys = dtc_chip_size_y / dtc_stat_bin;
	}

	if(dtc_no_transform == 1)
	  sprintf(body,
 "copy\nreply %d\nrow_mm %f\ncol_mm %f\ndist_mm %f\ntwo_theta %f\nheader_size %d\nrow_xfer %d\ncol_xfer %d\nrow_bin %d\ncol_bin %d\n",
		do_reply,
		dtc_stat_xcen,dtc_stat_ycen,dtc_stat_dist,dtc_stat_2theta,hsize,ys,xs,dtc_stat_bin,dtc_stat_bin);
	 else
	  sprintf(body,
 "xform\nreply %d\nrow_mm %f\ncol_mm %f\ndist_mm %f\ntwo_theta %f\nheader_size %d\nrow_xfer %d\ncol_xfer %d\nrow_bin %d\ncol_bin %d\n",
		do_reply,
		dtc_stat_xcen,dtc_stat_ycen,dtc_stat_dist,dtc_stat_2theta,hsize,ys,xs,dtc_stat_bin,dtc_stat_bin);

	if(dtc_stat_compress == 1)
		strcat(body,"compress 1\n");
	    else
		strcat(body,"compress 0\n");
	if(dtc_detector_sn > 0)
	  {
		sprintf(dzstuff,"detector_sn %d\n",dtc_detector_sn);
		strcat(body,dzstuff);
	  }
	sprintf(dzstuff,"save_raw %d\n",dtc_output_raws);
	strcat(body,dzstuff);

	switch(dtc_image_kind)
	  {
	    case KIND_DARK_0:
		strcpy(suffix,".dkc");
		break;
	    case KIND_DARK_1:
		strcpy(suffix,".dkc");
		break;
	    case KIND_DARK_2:
		strcpy(suffix,".dkd");
		break;
	    case KIND_DARK_3:
		strcpy(suffix,".dkd");
		break;
	    case KIND_RAW_0:
		strcpy(suffix,dtc_default_suffix);
		break;
	    case KIND_RAW_1:
		strcpy(suffix,dtc_default_suffix);
		break;
	  }
	sprintf(dzstuff,"dzratio %f\n",dtc_stat_dzratio);
	strcat(body, dzstuff);

	sprintf(dzstuff,"outfile_type %d\n",dtc_outfile_type);
	strcat(body,dzstuff);

	if(dtc_jpeg1_name[0] != '\0')
	{
		sprintf(dzstuff, "jpeg1_name %s\njpeg1_size %s\n", dtc_jpeg1_name, dtc_jpeg1_size);
		strcat(body, dzstuff);
		dtc_jpeg1_name[0] = '\0';
	}
	if(dtc_jpeg2_name[0] != '\0')
	{
		sprintf(dzstuff, "jpeg2_name %s\njpeg2_size %s\n", dtc_jpeg2_name, dtc_jpeg2_size);
		strcat(body, dzstuff);
		dtc_jpeg2_name[0] = '\0';
	}
	if(dtc_check_gather_return)
	{
		sprintf(dzstuff, "seq %s\n", file_seq);
		strcat(body, dzstuff);
	}
	if(dtc_mult_nframe > 1)
	{
		sprintf(dzstuff, "mult_frame_nframe %d\n", dtc_mult_nframe);
		strcat(body, dzstuff);
	}
		

	if(-1 == dtc_fdxfcm)
	{
		fprintf(stdout,"detcon_server: xform command file is NOT connected.\n");
		fprintf(stdout,"detcon_server: currently, THIS IS A WARNING\n");
		return(1);
	}
	else
	{
		merge_header_bytes = strlen(dtc_merge_header);
		sprintf(xfcmd_buf,"%sinfile <socket>\noutfile %s%s\nrawfile %s%s\nkind %d\nmerge_header_bytes %d\n",
			body,file_part1,suffix,file_part1,file_suffix,dtc_image_kind, merge_header_bytes);
		if(dtc_mult_nframe > 1)
		{
			sprintf(dzstuff, "mult_frame_nframe %d\n", dtc_mult_nframe);
			strcat(xfcmd_buf, dzstuff);
		}
		strcat(xfcmd_buf,"end_of_det\n");
		strcat(xfcmd_buf, dtc_merge_header);
		len_xfcmd = strlen(xfcmd_buf);
		dtc_merge_header[0] = '\0';		/* reset */

		pthread_mutex_lock(&dtc_h_mutex_fdxfcm);
		if(0)
		fprintf(stdout, "sending to image_gather:\n%s\n", xfcmd_buf);
		if(len_xfcmd != detcon_rep_write(dtc_fdxfcm,xfcmd_buf,len_xfcmd))
		{
			int 	save_errno;
			save_errno = errno;

			fprintf(stdout,"detcon_server: image_gather process has disconnected.\n");
			fprintf(stdout,"detcon_server: currently, THIS IS A WARNING.\n");
			detcon_notify_server_eof(dtc_fdxfcm, save_errno);
			pthread_mutex_unlock(&dtc_h_mutex_fdxfcm);
			return(1);
		}
		pthread_mutex_unlock(&dtc_h_mutex_fdxfcm);

		if(0)
		{
			fprintf(stderr,"output to ccd_image_gather with merge_header_bytes: %d\n", merge_header_bytes);
			write(2, xfcmd_buf, len_xfcmd);
			fprintf(stderr,"\n");
		}
	}
	return(0);
}


void	detcon_ccd_hw_initial_status()
{
	dtc_stat_start_phi = 0.;
	dtc_stat_start_omega = 0.;
	dtc_stat_start_kappa = 0.;
	dtc_stat_axis = 1;
	dtc_stat_osc_width = 1.0;
	dtc_stat_time = 30;
}

/*
 *	Temperature control routines.
 */

/*
 *	Make these values accessable for routines in this module for when the
 *	current temperature status has been "parsed" by any of the routines that
 *	have obtained the current temperature status.
 */

static	float	temp_read[9], temp_target[9], temp_final[9], temp_inc[9];

static	int	dtc_finished_ctr;

int	detcon_read_temperature()
{
	char	tempbuf[1024];
        int  	detret;

	sprintf(tempbuf, "temp_read\n");
	detret = detcon_output_detcmd(dtc_fdsecdetcmd, tempbuf, NULL, 0);

        if (detret == CCD_DET_NOTCONNECTED) 
	{
		setDTCState(DTC_STATE_ERROR, "in detcon_read_temperature: detector command process NOT connected", 0);
		return(1);
        }
	if(detret != CCD_DET_OK)
	{
		setDTCState(DTC_STATE_ERROR, "in detcon_read_temperature: ERROR reading detector temperature", 0);
		return(1);
        }

	detret = detcon_output_detcmd(dtc_fdsecdetcmd, "getparam\ntemp_read\n",NULL,0);

	if(0)
		fprintf(stdout,"detcon_read_temperature: returned string from getparam is:\n%s\n", dtc_det_reply);

	return(0);
}

int	detcon_print_temp_readings(char *readings)
{
	char	checkbuf[128];
	int	n_ctrl, n, done, stat, cold;
	int	temp_status[9];
	float	fval;
	char	*cp;

	cp = getenv("CCD_N_CTRL");
	n_ctrl = atoi(cp);
	for(n = 0; n < n_ctrl; n++)
		temp_status[n] = -1;

	for(n = 0; n < n_ctrl; n++)
	{
		sprintf(checkbuf, "temp_status %d", n);
		if(NULL != (cp = strstr(readings, checkbuf)))
		{
			sscanf(cp + strlen(checkbuf), "%d", &stat);
			temp_status[n] = stat;
		}
		sprintf(checkbuf, "temp_read %d", n);
		if(NULL != (cp = strstr(readings, checkbuf)))
		{
			sscanf(cp + strlen(checkbuf), "%f", &fval);
			temp_read[n] = fval;
		}
		sprintf(checkbuf, "temp_target %d", n);
		if(NULL != (cp = strstr(readings, checkbuf)))
		{
			sscanf(cp + strlen(checkbuf), "%f", &fval);
			temp_target[n] = fval;
		}
		sprintf(checkbuf, "temp_final %d", n);
		if(NULL != (cp = strstr(readings, checkbuf)))
		{
			sscanf(cp + strlen(checkbuf), "%f", &fval);
			temp_final[n] = fval;
		}
		sprintf(checkbuf, "increment_temp %d", n);
		if(NULL != (cp = strstr(readings, checkbuf)))
		{
			sscanf(cp + strlen(checkbuf), "%f", &fval);
			temp_inc[n] = fval;
		}
	}
	done = 1;
	for(n = 0; n < n_ctrl; n++)
		if(0 != temp_status[n])
			done = 0;
	cold = 1;
	for(n = 0; n < n_ctrl; n++)
		if(-60 > temp_read[n] || temp_read[n] > -40.)
			cold = 0;

	fprintf(stdout,"\n   stat         temp  target   final     inc\n");
	for(n = 0; n < n_ctrl; n++)
	{
		fprintf(stdout,"%d ", n);
		switch(temp_status[n])
		{
			case 0:
				fprintf(stdout,"@final temp");
				break;
			case 1:
				fprintf(stdout,"moving     ");
				break;
			case 2:
				fprintf(stdout,"stabilizing");
				break;
		}
		fprintf(stdout," %6.2f  %6.2f  %6.2f  %6.2f\n", temp_read[n], temp_target[n], temp_final[n], temp_inc[n]);
	}
	if(cold)
		fprintf(stdout,"\nDetector is COLD.\n");
	else
		fprintf(stdout,"\nOne or more modules of the detector are NOT cold.\n");

	return(0);
}
	
int	detcon_get_temperature_readings_fd(int fd)
{
	char	checkbuf[128];
	int	detret;
	int	n_ctrl, n, done, stat, cold, warm;
	int	temp_status[9];
	float	fval;
	char	*cp;

	detret = detcon_output_detcmd(fd, "getparam\ntemp_read\ntemp_target\ntemp_final\ntemp_increment\ntemp_status\n",NULL,0);

        if (detret == CCD_DET_NOTCONNECTED) 
	{
		setDTCState(DTC_STATE_ERROR, "in detcon_read_temperature: detector command process NOT connected", 0);
		return(1);
        }

	cp = getenv("CCD_N_CTRL");
	n_ctrl = atoi(cp);
	for(n = 0; n < n_ctrl; n++)
		temp_status[n] = -1;

	dtc_temp_status[0] = '\0';
	strcat_between(dtc_temp_status, dtc_det_reply, "OK\n", "end_of_det\n");

	if(0)
	{
		fprintf(stdout,"Temperature readings via secondary cmd socket:\n");
		detcon_print_temp_readings(dtc_temp_status);
	}

	for(n = 0; n < n_ctrl; n++)
		temp_status[n] = -1;

	for(n = 0; n < n_ctrl; n++)
	{
		sprintf(checkbuf, "temp_status %d", n);
		if(NULL != (cp = strstr(dtc_temp_status, checkbuf)))
		{
			sscanf(cp + strlen(checkbuf), "%d", &stat);
			temp_status[n] = stat;
		}
		sprintf(checkbuf, "temp_read %d", n);
		if(NULL != (cp = strstr(dtc_temp_status, checkbuf)))
		{
			sscanf(cp + strlen(checkbuf), "%f", &fval);
			temp_read[n] = fval;
		}
		sprintf(checkbuf, "temp_target %d", n);
		if(NULL != (cp = strstr(dtc_temp_status, checkbuf)))
		{
			sscanf(cp + strlen(checkbuf), "%f", &fval);
			temp_target[n] = fval;
		}
		sprintf(checkbuf, "temp_final %d", n);
		if(NULL != (cp = strstr(dtc_temp_status, checkbuf)))
		{
			sscanf(cp + strlen(checkbuf), "%f", &fval);
			temp_final[n] = fval;
		}
		sprintf(checkbuf, "increment_temp %d", n);
		if(NULL != (cp = strstr(dtc_temp_status, checkbuf)))
		{
			sscanf(cp + strlen(checkbuf), "%f", &fval);
			temp_inc[n] = fval;
		}
	}
	done = 1;
	for(n = 0; n < n_ctrl; n++)
		if(0 != temp_status[n])
			done = 0;
	cold = 1;
	for(n = 0; n < n_ctrl; n++)
		if(-60 > temp_read[n] || temp_read[n] > -40.)
			cold = 0;
	dtc_temp_cold = cold;

	warm = 1;
	for(n = 0; n < n_ctrl; n++)
		if(temp_read[n] < 5)
			warm = 0;
	dtc_temp_warm = warm;
	
	dtc_temps_transferred = 1;

	return(0);
}

int	detcon_temp_read_for_idle(int n_reads)
{
	int	detret;
	int	n;

	for(n = 0; n < n_reads; n++)
	{
		detret = detcon_output_detcmd(dtc_fdsecdetcmd, "temp_read\n", NULL, 0);
		detcon_sleep(1000);
	}

	return(0);
}

int	detcon_temp_read_for_stable(int n_reads)
{
	int	detret;
	int	n;

	for(n = 0; n < n_reads; n++)
	{
		detret = detcon_output_detcmd(dtc_fdsecdetcmd, "temp_read\n", NULL, 0);
		detcon_sleep(1000);
	}

	return(0);
}

int	detcon_temp_idle_when_finished(int val)
{
	char	checkbuf[128];
	int	detret;
	int	n_ctrl, n, done, stat;
	int	temp_status[9];
	float	fval;
	char	*cp;

	done = 0;
	while(!done)
	{
		cp = getenv("CCD_N_CTRL");
		n_ctrl = atoi(cp);
		for(n = 0; n < n_ctrl; n++)
			temp_status[n] = -1;
	
		detret = detcon_output_detcmd(dtc_fdsecdetcmd, "temp_read\n", NULL, 0);
		detret = detcon_output_detcmd(dtc_fdsecdetcmd, "getparam\ntemp_status\ntemp_read\ntemp_target\ntemp_final\ntemp_increment\n", NULL, 0);
	
		fprintf(stdout,"detcon_temp_idle_when_finished: dtc_det_reply:\n%s", dtc_det_reply);
	
		for(n = 0; n < n_ctrl; n++)
		{
			sprintf(checkbuf, "temp_status %d", n);
			if(NULL != (cp = strstr(dtc_det_reply, checkbuf)))
			{
				sscanf(cp + strlen(checkbuf), "%d", &stat);
				temp_status[n] = stat;
			}
			sprintf(checkbuf, "temp_read %d", n);
			if(NULL != (cp = strstr(dtc_det_reply, checkbuf)))
			{
				sscanf(cp + strlen(checkbuf), "%f", &fval);
				temp_read[n] = fval;
			}
			sprintf(checkbuf, "temp_target %d", n);
			if(NULL != (cp = strstr(dtc_det_reply, checkbuf)))
			{
				sscanf(cp + strlen(checkbuf), "%f", &fval);
				temp_target[n] = fval;
			}
			sprintf(checkbuf, "temp_final %d", n);
			if(NULL != (cp = strstr(dtc_det_reply, checkbuf)))
			{
				sscanf(cp + strlen(checkbuf), "%f", &fval);
				temp_final[n] = fval;
			}
			sprintf(checkbuf, "increment_temp %d", n);
			if(NULL != (cp = strstr(dtc_det_reply, checkbuf)))
			{
				sscanf(cp + strlen(checkbuf), "%f", &fval);
				temp_inc[n] = fval;
			}
		}
		done = 1;
		for(n = 0; n < n_ctrl; n++)
			if(0 != temp_status[n])
				done = 0;
	
		fprintf(stdout,"\n stat         temp  target   final     inc\n");
		for(n = 0; n < n_ctrl; n++)
		{
			switch(temp_status[n])
			{
				case 0:
					fprintf(stdout,"@final temp");
					break;
				case 1:
					fprintf(stdout,"moving     ");
					break;
				case 2:
					fprintf(stdout,"stabilizing");
					break;
			}
			fprintf(stdout," %6.2f  %6.2f  %6.2f  %6.2f\n", temp_read[n], temp_target[n], temp_final[n], temp_inc[n]);
		}
	
		dtc_finished_ctr--;
		if(dtc_finished_ctr <= 0)
			dtc_finished_ctr = 0;
	
		if(dtc_finished_ctr > 0)
			detcon_temp_read_for_idle(5);
		else
			break;
	}
	detret = detcon_output_detcmd(dtc_fdsecdetcmd, "hwreset\n", NULL, 0);
	
	return(0);
}

int	detcon_temp_stable_when_finished(int val)
{
	char	checkbuf[128];
	int	detret;
	int	n_ctrl, n, done, stat;
	int	temp_status[9];
	float	fval;
	char	*cp;

	done = 0;
	while(!done)
	{
		cp = getenv("CCD_N_CTRL");
		n_ctrl = atoi(cp);
		for(n = 0; n < n_ctrl; n++)
			temp_status[n] = -1;
	
		detret = detcon_output_detcmd(dtc_fdsecdetcmd, "temp_read\n", NULL, 0);
		detret = detcon_output_detcmd(dtc_fdsecdetcmd, "getparam\ntemp_status\ntemp_read\ntemp_target\ntemp_final\ntemp_increment\n", NULL, 0);
	
		fprintf(stdout,"detcon_temp_stable_when_finished: dtc_det_reply:\n%s", dtc_det_reply);
	
		for(n = 0; n < n_ctrl; n++)
		{
			sprintf(checkbuf, "temp_status %d", n);
			if(NULL != (cp = strstr(dtc_det_reply, checkbuf)))
			{
				sscanf(cp + strlen(checkbuf), "%d", &stat);
				temp_status[n] = stat;
			}
			sprintf(checkbuf, "temp_read %d", n);
			if(NULL != (cp = strstr(dtc_det_reply, checkbuf)))
			{
				sscanf(cp + strlen(checkbuf), "%f", &fval);
				temp_read[n] = fval;
			}
			sprintf(checkbuf, "temp_target %d", n);
			if(NULL != (cp = strstr(dtc_det_reply, checkbuf)))
			{
				sscanf(cp + strlen(checkbuf), "%f", &fval);
				temp_target[n] = fval;
			}
			sprintf(checkbuf, "temp_final %d", n);
			if(NULL != (cp = strstr(dtc_det_reply, checkbuf)))
			{
				sscanf(cp + strlen(checkbuf), "%f", &fval);
				temp_final[n] = fval;
			}
			sprintf(checkbuf, "increment_temp %d", n);
			if(NULL != (cp = strstr(dtc_det_reply, checkbuf)))
			{
				sscanf(cp + strlen(checkbuf), "%f", &fval);
				temp_inc[n] = fval;
			}
		}
		done = 1;
		for(n = 0; n < n_ctrl; n++)
			if(0 != temp_status[n])
				done = 0;
	
		fprintf(stdout,"\n stat         temp  target   final     inc\n");
		for(n = 0; n < n_ctrl; n++)
		{
			switch(temp_status[n])
			{
				case 0:
					fprintf(stdout,"@final temp");
					break;
				case 1:
					fprintf(stdout,"moving     ");
					break;
				case 2:
					fprintf(stdout,"stabilizing");
					break;
			}
			fprintf(stdout," %6.2f  %6.2f  %6.2f  %6.2f\n", temp_read[n], temp_target[n], temp_final[n], temp_inc[n]);
		}
	
		dtc_finished_ctr--;
		if(dtc_finished_ctr <= 0)
			dtc_finished_ctr = 0;
	
		if(dtc_finished_ctr > 0)
			detcon_temp_read_for_stable(5);
		else
			break;
	}
	return(0);
}

void	*detcon_set_temperature(void *arg)
{
	char	tempbuf[1024];
        int  	detret;
	double	val;

	val = dtc_temp_value;
	sprintf(tempbuf, "temp_set\ntemp_set %.3f\n", val);
	detret = detcon_output_detcmd(dtc_fdsecdetcmd, tempbuf, NULL, 0);

        if (detret == CCD_DET_NOTCONNECTED) 
	{
		setDTCState(DTC_STATE_ERROR, "in detcon_set_temperature: detector command process NOT connected", 0);
		return(NULL);
        }

	dtc_finished_ctr = 6;

	detcon_temp_idle_when_finished(0);

	setDTCState(DTC_STATE_IDLE, "in detcon_temp_stable_when_finished: state set to IDLE on completion", 0);

	return(NULL);
}

/*
 *	Ramp, meaning move in 5 degree increments with a 5 minute stabilization for
 *	each increment, from the current temperature to the target temperature
 *	given by dtc_temp_value.
 *
 *	Since the movement of 5 degrees when the detector is warmer than about
 *	+10 degrees C is sluggish to the point of being non-responsive, we read
 *	the current detector temperature and if any module is warmer than +5
 *	we do a "set" of the temperature to 0.0 degrees, wait for stability, and
 *	then ramp to the target temperature.
 */

void	*detcon_ramp_temperature(void *arg)
{
	char	tempbuf[1024];
	int	any_warm;
	char	*cp;
	int	n_ctrl;
	int	n;
	int	detret;
	double	val;

	/*
	 *	First figure out what our current temperature is.
	 */

	detcon_temp_read_for_stable(5);

	detcon_get_temperature_readings_fd(dtc_fdsecdetcmd);

	detcon_print_temp_readings(dtc_temp_status);

	cp = getenv("CCD_N_CTRL");
	n_ctrl = atoi(cp);
	fprintf(stdout, "detcon_ramp_temperatures: above temperature readings are used to make set decision.\n");
	any_warm = 0;
	for(n = 0; n < n_ctrl; n++)
	{
		if(temp_read[n] > 5)
			any_warm = 1;
		fprintf(stdout,"  temp_read[%d]: %8.2f any_warm: %d\n", n, temp_read[n], any_warm);
	}

	if(1 == any_warm && dtc_temp_value < 5)
	{
		if(dtc_temp_value < 0)
			val = 0;
		else
			val = dtc_temp_value;
		sprintf(tempbuf, "temp_set\ntemp_set %.3f\n", val);
		detret = detcon_output_detcmd(dtc_fdsecdetcmd, tempbuf, NULL, 0);

        	if (detret == CCD_DET_NOTCONNECTED) 
		{
			setDTCState(DTC_STATE_ERROR, "in detcon_set_temperature: detector command process NOT connected", 0);
			return(NULL);
        	}

		dtc_finished_ctr = 100;

		detcon_temp_idle_when_finished(0);
	}

	val = dtc_temp_value;
	sprintf(tempbuf, "temp_ramp\ntemp_ramp %.3f\n", val);
	detret = detcon_output_detcmd(dtc_fdsecdetcmd, tempbuf, NULL, 0);

        if (detret == CCD_DET_NOTCONNECTED) 
	{
		setDTCState(DTC_STATE_ERROR, "in detcon_ramp_temperature: detector command process NOT connected", 0);
		return(NULL);
        }

	dtc_finished_ctr = 10000;

	detcon_temp_idle_when_finished(0);

	setDTCState(DTC_STATE_IDLE, "in detcon_ramp_temperature: state set to IDLE on completion", 0);

	return(NULL);
}

int	detcon_abort_temperature()
{
        int  	detret;

	detret = detcon_output_detcmd(dtc_fdsecdetcmd, "abort_temp\n", NULL, 0);

        if (detret == CCD_DET_NOTCONNECTED) 
	{
		setDTCState(DTC_STATE_ERROR, "in detcon_abort_temperature: detector command process NOT connected", 0);
		return(1);
        }

	dtc_finished_ctr = 3;

	detcon_temp_idle_when_finished(0);

	return(0);
}

int	detcon_get_stable_temperature_readings()
{
        int  	detret;

	detret = detcon_output_detcmd(dtc_fdsecdetcmd, "temp_read\n", NULL, 0);

        if (detret == CCD_DET_NOTCONNECTED) 
	{
		setDTCState(DTC_STATE_ERROR, "in detcon_get_stable_temperature: detector command process NOT connected", 0);
		return(1);
        }

	setDTCState(DTC_STATE_TEMPCONTROL, "in detcon_get_stable_temperature: state set to TEMPCONTROL", 0);
	dtc_finished_ctr = 3;

	detcon_temp_stable_when_finished(0);

	return(0);
}

