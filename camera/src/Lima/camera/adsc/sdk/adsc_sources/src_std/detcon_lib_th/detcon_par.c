#include	"detcon_ext.h"

static int	debug_hwp_par = 0;
static int	debug_flp_par = 0;

struct dtc_parstruct {
			int	dtc_parno;
			int	dtc_partype;
			char	*dtc_parptr;
			int	*dtc_parset;
			int	(*dtc_parfcn)(int);
		     };

#define	DTC_TYPE_INT	0
#define	DTC_TYPE_FLOAT	1
#define	DTC_TYPE_STRING 2

struct dtc_parstruct hwp_param[] = {
	{ HWP_BIN, 	DTC_TYPE_INT,	(char *) &dtc_stat_bin,		NULL,	NULL },
	{ HWP_ADC,	DTC_TYPE_INT,	(char *) &dtc_stat_adc,		NULL,	NULL },
	{ HWP_SAVE_RAW,	DTC_TYPE_INT,	(char *) &dtc_output_raws,	NULL,	NULL },
	{ HWP_DARK,	DTC_TYPE_INT,	(char *) &dtc_force_dark,	NULL,	NULL },
	{ HWP_STORED_DARK,DTC_TYPE_INT,	(char *) &dtc_stored_dark,	NULL,	NULL },
	{ HWP_LOADFILE,	DTC_TYPE_INT,	(char *) &dtc_loadfile,		NULL,	NULL },
	{ HWP_NO_XFORM,	DTC_TYPE_INT,	(char *) &dtc_no_transform,	NULL,	NULL },
	{ HWP_TEMP_COLD,	DTC_TYPE_INT,	(char *) &dtc_temp_cold,	NULL,	detcon_temp_par_cold },
	{ HWP_TEMP_WARM,	DTC_TYPE_INT,	(char *) &dtc_temp_warm,	NULL,	detcon_temp_par_warm },
	{ HWP_TEMP_MODE,	DTC_TYPE_INT,	(char *) &dtc_temp_mode,	NULL,	detcon_temp_par_mode },
	{ HWP_TEMP_STATUS,DTC_TYPE_STRING,(char *) dtc_temp_status,	NULL,	detcon_temp_par_status },
	{ HWP_TEMP_VALUE, DTC_TYPE_FLOAT, (char *) &dtc_temp_value,	NULL,	detcon_temp_par_value },
	{ HWP_MULT_TRIGTYPE,	DTC_TYPE_INT,	(char *) &dtc_mult_trigtype,	NULL,	NULL },
	{ HWP_MULT_FRAME_START,	DTC_TYPE_INT,	(char *) &dtc_mult_frame_start,	NULL,	NULL },
	{ HWP_MULT_NFRAME,	DTC_TYPE_INT,	(char *) &dtc_mult_nframe,	NULL,	NULL },
	{ HWP_MULT_SETTRIGGER,	DTC_TYPE_INT,	(char *) &dtc_mult_settrigger,	NULL,	NULL },
	{ 0,		0,		NULL,				NULL,	NULL }
     };

struct dtc_parstruct flp_param[] = {
	{ FLP_PHI,		DTC_TYPE_FLOAT,		(char *) &dtc_stat_phi,		NULL,			NULL },
	{ FLP_OMEGA,		DTC_TYPE_FLOAT,		(char *) &dtc_stat_omega,	NULL,			NULL },
	{ FLP_KAPPA,		DTC_TYPE_FLOAT,		(char *) &dtc_stat_kappa,	&dtc_wasset_kappa,	NULL },
	{ FLP_TWOTHETA,		DTC_TYPE_FLOAT,		(char *) &dtc_stat_2theta,	&dtc_wasset_2theta,	NULL },
	{ FLP_DISTANCE,		DTC_TYPE_FLOAT,		(char *) &dtc_stat_dist,	NULL,			NULL },
	{ FLP_WAVELENGTH,		DTC_TYPE_FLOAT,		(char *) &dtc_stat_wavelength,	&dtc_wasset_wavelength,	NULL },
	{ FLP_AXIS,		DTC_TYPE_INT,		(char *) &dtc_stat_axis,	NULL,			NULL },
	{ FLP_OSC_RANGE,		DTC_TYPE_FLOAT,		(char *) &dtc_stat_osc_width,	NULL,			NULL },
	{ FLP_TIME,		DTC_TYPE_FLOAT,		(char *) &dtc_stat_time,	NULL,			NULL },
	{ FLP_DOSE,		DTC_TYPE_FLOAT,		(char *) &dtc_stat_dose,	NULL,			NULL },
	{ FLP_BEAM_X,		DTC_TYPE_FLOAT,		(char *) &dtc_stat_xcen,	NULL,			NULL },
	{ FLP_BEAM_Y,		DTC_TYPE_FLOAT,		(char *) &dtc_stat_ycen,	NULL,			NULL },
	{ FLP_COMPRESS,		DTC_TYPE_INT,		(char *) &dtc_stat_compress,	NULL,			NULL },
	{ FLP_KIND,		DTC_TYPE_INT,		(char *) &dtc_image_kind,	NULL,			NULL },
	{ FLP_FILENAME,		DTC_TYPE_STRING,	dtc_filename,			NULL,			NULL },
	{ FLP_COMMENT,		DTC_TYPE_STRING,	dtc_comment,			NULL,			NULL },
	{ FLP_LASTIMAGE,		DTC_TYPE_INT,		(char *) &dtc_lastimage,	NULL,			NULL },
        { FLP_SUFFIX,		DTC_TYPE_STRING,	dtc_default_suffix,		NULL,			NULL },
        { FLP_IMBYTES, 		DTC_TYPE_INT,		(char *) &dtc_default_imsize,	NULL,			NULL },
	{ FLP_READ_FILENAME,	DTC_TYPE_STRING,	dtc_read_filename,		NULL,			NULL },
	{ FLP_USERDEF_STR,	DTC_TYPE_STRING,	dtc_userdef_str,		NULL,			NULL },
	{ FLP_USERRET,		DTC_TYPE_STRING,	dtc_det_reply,			NULL,			NULL },
	{ FLP_HEADER,		DTC_TYPE_STRING,	dtc_merge_header,		NULL,			NULL },
	{ FLP_JPEG1_NAME,		DTC_TYPE_STRING,	dtc_jpeg1_name,			NULL,			NULL },
	{ FLP_JPEG1_SIZE,		DTC_TYPE_STRING,	dtc_jpeg1_size,			NULL,			NULL },
	{ FLP_JPEG2_NAME,		DTC_TYPE_STRING,	dtc_jpeg2_name,			NULL,			NULL },
	{ FLP_JPEG2_SIZE,		DTC_TYPE_STRING,	dtc_jpeg2_size,			NULL,			NULL },
	{ FLP_OUTFILE_TYPE,	DTC_TYPE_INT,		(char *) &dtc_outfile_type,	NULL,			NULL },
	{ FLP_HEADERPARAMS,	DTC_TYPE_STRING,	dtc_headerparams,		NULL,			NULL },
	{ 0,			0,			NULL,				NULL,			NULL }
     };

int	detcon_set_hw_param(int which_par, char *p_value)
{
	int	i;

	for(i = 0; NULL != hwp_param[i].dtc_parptr; i++)
	  if(which_par == hwp_param[i].dtc_parno)
	    {
		switch(hwp_param[i].dtc_partype)
		  {
		    case DTC_TYPE_INT:
			*((int *) hwp_param[i].dtc_parptr) = *((int *) p_value);
			if(debug_hwp_par)
			fprintf(stderr,
			  "detcon_set_hw_param: (int)    set param %2d to %d\n",
				which_par,*((int *) hwp_param[i].dtc_parptr));
			break;
		    case DTC_TYPE_FLOAT:
			*((float *) hwp_param[i].dtc_parptr) = *((float *) p_value);
			if(debug_hwp_par)
			fprintf(stderr,
			  "detcon_set_hw_param: (float)  set param %2d to %f\n",
				which_par,*((float *) hwp_param[i].dtc_parptr));
			break;
		    case DTC_TYPE_STRING:
			if(NULL == (char *) p_value)
				*hwp_param[i].dtc_parptr = '\0';
			  else
				strcpy((char *) hwp_param[i].dtc_parptr,(char *) p_value);
			if(debug_hwp_par)
			fprintf(stderr,
			  "detcon_set_hw_param: (string) set param %2d to %s\n",
				which_par,(char *) hwp_param[i].dtc_parptr);
			break;
		  }
		if(NULL != hwp_param[i].dtc_parset)
			*(hwp_param[i].dtc_parset) = 1;
		if(NULL != hwp_param[i].dtc_parfcn)
			(*hwp_param[i].dtc_parfcn)(1);
		return(0);
	  }
	return(1);
}

int	detcon_set_file_param(int which_par, char *p_value)
{
	int	i;

	for(i = 0; NULL != flp_param[i].dtc_parptr; i++)
	  if(which_par == flp_param[i].dtc_parno)
	    {
		switch(flp_param[i].dtc_partype)
		  {
		    case DTC_TYPE_INT:
			*((int *) flp_param[i].dtc_parptr) = *((int *) p_value);
			if(debug_flp_par)
			fprintf(stderr,
			  "detcon_set_file_param: (int)    set param %2d to %d\n",
				which_par,*((int *) flp_param[i].dtc_parptr));
			break;
		    case DTC_TYPE_FLOAT:
			*((float *) flp_param[i].dtc_parptr) = *((float *) p_value);
			if(debug_flp_par)
			fprintf(stderr,
			  "detcon_set_file_param: (float)  set param %2d to %f\n",
				which_par,*((float *) flp_param[i].dtc_parptr));
			break;
		    case DTC_TYPE_STRING:
			if(NULL == (char *) p_value)
				*flp_param[i].dtc_parptr = '\0';
			  else
				strcpy((char *) flp_param[i].dtc_parptr,(char *) p_value);
			if(debug_flp_par)
			fprintf(stderr,
			  "detcon_set_file_param: (string) set param %2d to %s\n",
				which_par,(char *) flp_param[i].dtc_parptr);
			break;
		  }
		if(NULL != flp_param[i].dtc_parset)
			*(flp_param[i].dtc_parset) = 1;
		return(0);
	  }
	return(1);
}

int	detcon_get_hw_param(int which_par,char *p_value)
{
	int	i;

	for(i = 0; NULL != hwp_param[i].dtc_parptr; i++)
	  if(which_par == hwp_param[i].dtc_parno)
	    {
		if(NULL != hwp_param[i].dtc_parfcn)
			(*hwp_param[i].dtc_parfcn)(0);

		switch(hwp_param[i].dtc_partype)
		  {
		    case DTC_TYPE_INT:
			*((int *) p_value) = *((int *) hwp_param[i].dtc_parptr);
			break;
		    case DTC_TYPE_FLOAT:
			*((float *) p_value) = *(float *) hwp_param[i].dtc_parptr;
			break;
		    case DTC_TYPE_STRING:
			strcpy((char *) p_value, (char *) hwp_param[i].dtc_parptr);
			break;
		  }

		return(0);
	  }
	return(1);
}

int	detcon_get_file_param(int which_par,char *p_value)
{
	int	i;

	if(dtc_stat_bin == 1)
		dtc_default_imsize = 2 * 4096 * 4096;
	else
		dtc_default_imsize = 2 * 2048 * 2048;

	for(i = 0; NULL != flp_param[i].dtc_parptr; i++)
	  if(which_par == flp_param[i].dtc_parno)
	    {
		switch(flp_param[i].dtc_partype)
		  {
		    case DTC_TYPE_INT:
			*((int *) p_value) = *((int *) flp_param[i].dtc_parptr);
			break;
		    case DTC_TYPE_FLOAT:
			*((float *) p_value) = *((float *) flp_param[i].dtc_parptr);
			break;
		    case DTC_TYPE_STRING:
			strcpy((char *) p_value, (char *) flp_param[i].dtc_parptr);
			break;
		  }
		return(0);
	  }
	return(1);
}

int	str_detcon_set_hw_param(int which_par, char *p_value)
{
	int	i;
	double	atof();
	int	atoi();

	for(i = 0; NULL != hwp_param[i].dtc_parptr; i++)
	  if(which_par == hwp_param[i].dtc_parno)
	    {
		switch(hwp_param[i].dtc_partype)
		  {
		    case DTC_TYPE_INT:
			*((int *) hwp_param[i].dtc_parptr) = atoi(p_value);
			if(debug_hwp_par)
			fprintf(stderr,
			  "detcon_set_hw_param: (int)    set param %2d to %d\n",
				which_par,*((int *) hwp_param[i].dtc_parptr));
			break;
		    case DTC_TYPE_FLOAT:
			*((float *) hwp_param[i].dtc_parptr) = atof(p_value);
			if(debug_hwp_par)
			fprintf(stderr,
			  "detcon_set_hw_param: (float)  set param %2d to %f\n",
				which_par,*((float *) hwp_param[i].dtc_parptr));
			break;
		    case DTC_TYPE_STRING:
			if(NULL == (char *) p_value)
				*hwp_param[i].dtc_parptr = '\0';
			  else
				strcpy((char *) hwp_param[i].dtc_parptr,(char *) p_value);
			if(debug_hwp_par)
			fprintf(stderr,
			  "detcon_set_hw_param: (string) set param %2d to %s\n",
				which_par,(char *) hwp_param[i].dtc_parptr);
			break;
		  }
		if(NULL != hwp_param[i].dtc_parset)
			*(hwp_param[i].dtc_parset) = 1;
		if(NULL != hwp_param[i].dtc_parfcn)
			(*hwp_param[i].dtc_parfcn)(1);
		return(0);
	  }
	return(1);
}

int	str_detcon_set_file_param(int which_par, char *p_value)
{
	int	i;
	double	atof();
	int	atoi();

	for(i = 0; NULL != flp_param[i].dtc_parptr; i++)
	  if(which_par == flp_param[i].dtc_parno)
	    {
		switch(flp_param[i].dtc_partype)
		  {
		    case DTC_TYPE_INT:
			*((int *) flp_param[i].dtc_parptr) = atoi(p_value);
			if(debug_flp_par)
			fprintf(stderr,
			  "detcon_set_file_param: (int)    set param %2d to %d\n",
				which_par,*((int *) flp_param[i].dtc_parptr));
			break;
		    case DTC_TYPE_FLOAT:
			*((float *) flp_param[i].dtc_parptr) = atof(p_value);
			if(debug_flp_par)
			fprintf(stderr,
			  "detcon_set_file_param: (float)  set param %2d to %f\n",
				which_par,*((float *) flp_param[i].dtc_parptr));
			break;
		    case DTC_TYPE_STRING:
			if(NULL == (char *) p_value)
				*flp_param[i].dtc_parptr = '\0';
			  else
				strcpy((char *) flp_param[i].dtc_parptr,(char *) p_value);
			if(debug_flp_par)
			fprintf(stderr,
			  "detcon_set_file_param: (string) set param %2d to %s\n",
				which_par,(char *) flp_param[i].dtc_parptr);
			break;
		  }
		if(NULL != flp_param[i].dtc_parset)
			*(flp_param[i].dtc_parset) = 1;
		return(0);
	  }
	return(1);
}


int	str_detcon_get_hw_param(int which_par,char *p_value)
{
	int	i;

	for(i = 0; NULL != hwp_param[i].dtc_parptr; i++)
	  if(which_par == hwp_param[i].dtc_parno)
	    {
		if(NULL != hwp_param[i].dtc_parfcn)
			(*hwp_param[i].dtc_parfcn)(0);
		switch(hwp_param[i].dtc_partype)
		  {
		    case DTC_TYPE_INT:
			sprintf(p_value,"%d",*((int *) hwp_param[i].dtc_parptr));
			break;
		    case DTC_TYPE_FLOAT:
			sprintf(p_value,"%f",*(float *) hwp_param[i].dtc_parptr);
			break;
		    case DTC_TYPE_STRING:
			strcpy(p_value, (char *) hwp_param[i].dtc_parptr);
			break;
		  }
		return(0);
	  }
	return(1);
}

int	str_detcon_get_file_param(int which_par, char *p_value)
{
	int	i;

	for(i = 0; NULL != flp_param[i].dtc_parptr; i++)
	  if(which_par == flp_param[i].dtc_parno)
	    {
		switch(flp_param[i].dtc_partype)
		  {
		    case DTC_TYPE_INT:
			sprintf(p_value,"%d",*((int *) flp_param[i].dtc_parptr));
			break;
		    case DTC_TYPE_FLOAT:
			sprintf(p_value,"%f",*((float *) flp_param[i].dtc_parptr));
			break;
		    case DTC_TYPE_STRING:
			strcpy(p_value, (char *) flp_param[i].dtc_parptr);
			break;
		  }
		return(0);
	  }
	return(1);
}
