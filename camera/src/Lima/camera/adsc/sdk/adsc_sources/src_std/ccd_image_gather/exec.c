#include	"ext.h"

/*
 *	Module to handle the execution of commands.
 */

static char reply_term[] = REPLY_TERM_STRING;

void	reply_cmd(char *reply, char *info)
{
	int 	len;

	if(0)
		fprintf(stderr,"reply_cmd: reply: %s info:%s\n", reply, info);

	if(reply_to_sender == 0)
		return;

	len = strlen(reply);
	if(-1 == rep_write(command_fd,reply,len))
	{
		notify_server_eof(command_fd, errno);
		return;
	}
	if(info != NULL)
	{
		len = strlen(info);
		if(-1 == rep_write(command_fd,info,len))
		{
			notify_server_eof(command_fd, errno);
			return;
		}
	}
	len = strlen(reply_term);
	if(-1 == rep_write(command_fd,reply_term,len))
	{
		notify_server_eof(command_fd, errno);
		return;
	}
}

/*
 *	Execute command "command_number" with parameters as set in the globals.
 */

int	execute_command()
{
	int 	retval;
	int	save_error;
	char	*cp;
	int	save_type;
	int	seq_err;
	char	seq_string[1024];
	char	err_buf[1024];

	retval = 0;
	save_type = outfile_type;
	seq_err = 0;

	switch(command_number)
	  {
	    /*
	     *	XFORM and COPY use the same basic code.
	     */

	    case XFORM_CMD:
	    case COPY_CMD:
		switch(image_kind)
		  {
		    case KIND_DARK_0:
		    case KIND_DARK_1:
		    case KIND_DARK_2:
		    case KIND_DARK_3:
			dkc_data_size[image_kind - KIND_DARK_0] = ccd_row_xfersize * ccd_col_xfersize;
			dkc_header_size[image_kind - KIND_DARK_0] = input_header_size;
			dkc_seen[image_kind - KIND_DARK_0] = 1;

			save_error = 0;
			seq_err = 0;
			if(save_raw_images)
			{
				receive_data_raw(image_kind);
				if(file_seq[0] != '\0')
				{
					seq_string[0] = '\0';
					gethd("DATE", seq_string, dkc_header[image_kind]);
					if(seq_string[0] != '\0' && file_seq[0] != '\0')
						if(0 != strcmp(seq_string, file_seq))
							seq_err = 1;
				}
				save_error = save_image(image_kind,0,compress_mode);
			}
			if(save_error)
				reply_cmd(REPLY_ERROR_STRING, "Error saving image");
			else if(seq_err)
			{
				sprintf(err_buf, "Sequence error: header_seq: %s command_seq: %s DO NOT MATCH ", seq_string, file_seq);
				reply_cmd(REPLY_ERROR_STRING, err_buf);
			}
			else
				reply_cmd(REPLY_OK_STRING,NULL);
			break;

		    case KIND_RAW_0:
		    case KIND_RAW_1:
			raw_data_size[image_kind - KIND_RAW_0] = ccd_row_xfersize * ccd_col_xfersize;
			raw_header_size[image_kind - KIND_RAW_0] = input_header_size;

			raw_seen[image_kind - KIND_RAW_0] = 1;
			save_error = 0;
			seq_err = 0;
			if(save_raw_images)
			{
				receive_data_raw(image_kind);
				if(file_seq[0] != '\0')
				{
					seq_string[0] = '\0';
					gethd("DATE", seq_string, raw_header[image_kind - KIND_RAW_0]);
					if(seq_string[0] != '\0' && file_seq[0] != '\0')
						if(0 != strcmp(seq_string, file_seq))
							seq_err = 1;
				}
				save_error = save_image(image_kind,0,compress_mode);
			}

			if(command_number == XFORM_CMD && image_kind == KIND_RAW_1)
			{

				receive_data_cor(KIND_RAW_1);
				if(file_seq[0] != '\0')
				{
					seq_string[0] = '\0';
					gethd("DATE", seq_string, raw_header[image_kind - KIND_RAW_0]);
					if(seq_string[0] != '\0' && file_seq[0] != '\0')
						if(0 != strcmp(seq_string, file_seq))
							seq_err = 1;
				}

				/*
				 *	The file save type (img, cbf, or both) is given by the
				 *	outfile_type parameter passed to ccd_image_gather.  If
				 *	also specified through the environment, the environment
				 *	setting will control.
				 */

				if(NULL != (cp = getenv("CCD_OUTPUT_CBF_FILES")))
				{
					if(0 == strcmp(cp, "only") || 0 == strcmp(cp, "ONLY"))
						save_type = OUTFILE_CBF_ONLY;
					else if(0 == strcmp(cp, "both") || 0 == strcmp(cp, "BOTH"))
						save_type = OUTFILE_IMG_CBF;
					else
						save_type = OUTFILE_IMG_ONLY;
				}

				switch(save_type)
				{
					case OUTFILE_IMG_ONLY:
						save_error = save_image_with_convert_smv(compress_mode);
						break;
					case OUTFILE_CBF_ONLY:
						save_error = save_image_with_convert_cbf(compress_mode);
						break;
					case OUTFILE_IMG_CBF:
						save_error = save_image_with_convert_smv(compress_mode);
						save_error = save_image_with_convert_cbf(compress_mode);
						break;
					default:
						fprintf(stderr,
						"\n\nccd_image_gather: WARNING: outfile_type %d is not known; defaulting to .img file type output\n\n", save_type);
						save_error = save_image_with_convert_smv(compress_mode);
						break;
				}
			}
			if(save_error)
				reply_cmd(REPLY_ERROR_STRING, "Error saving image");
			else if(seq_err)
			{
				sprintf(err_buf, "Sequence error: header_seq: %s command_seq: %s DO NOT MATCH ", seq_string, file_seq);
				reply_cmd(REPLY_ERROR_STRING, err_buf);
			}
			else
				reply_cmd(REPLY_OK_STRING,NULL);
			break;
		  }
		break;
	    case RESET_CMD:
		initial_defaults();
        	ccd_bufind = 0;
        	raw_header_size[0] = 0;
        	raw_header_size[1] = 0;
        	ccd_data_valid[0] = 0;
        	ccd_data_valid[1] = 0;
		reply_cmd(REPLY_OK_STRING,NULL);
		break;
	    case SETPARAM_CMD:
		reply_cmd(REPLY_OK_STRING,NULL);
		break;
	    case GETPARAM_CMD:
		reply_cmd(REPLY_OK_STRING,replybuf);
		break;
	    case STATUS_CMD:
		get_xform_status(replybuf);
		reply_cmd(REPLY_OK_STRING,replybuf);
		break;
	    case EXIT_CMD:
		retval = 1;
		break;
	  }
	file_seq[0] = '\0';
	return(retval);
}

/*
 *	Turn an integer into xxx.  Used in image numbers.
 */

void	util_3digit(char *s1, int val)
{
	int 	i,j;

	i = val;
	j = i / 100;
	*s1++ = (char ) ('0' + j);
	i = i - 100 * j;
	j = i / 10;
	*s1++ = (char ) ('0' + j);
	i = i - 10 * j;
	*s1++ = (char ) ('0' + i);
	*s1++ = '\0';
}
