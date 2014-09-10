#include	"detcon_ext.h"

/*
 *-------------------------------------------
 *
 *	Miscellaneous routines in this module.
 *
 *-------------------------------------------
 */

/*
 *	ccd_initialize
 *
 *	ccd_initialize translates logical names
 *	into actual file names (environment for
 *	UNIX, logical name table for VMS) and
 *	opens them in their appropriate states.
 *
 *	Next, ccd_initialize assigns default values
 *	of scanner times or attributes to the
 *	variables labled "specific...".
 *
 *	Then ccd_initialize reads the user's
 *	configuration file and alters the
 *	"specific..." variables based on that
 *	file's contents.
 *
 *	Finally, the "specific..." variables are
 *	used to assign values to variables which
 *	the scanner actually uses.
 *
 *	So the order of precedence:
 *
 *	  1)	mardefs.h	(lowest)
 *	  2)	user config	(next)
 *	  3)	values from profile (highest)
 */

int	detcon_ccd_initialize()
{
	long	clock;
	char	*cptr;

	dtc_merge_header[0] = '\0';

	if(NULL == (dtc_fpnull = fopen("/dev/null","r+")))
	  {
	    fprintf(stderr,"ccd_dc: ccd_initialize: cannot open /dev/null\n");
	    exit(0);
	  }

	detcon_ccd_init_files();
/*
 *	Log the time to the errorlog for startup
 */
	time(&clock);
	cptr = (char *) ctime(&clock);
	fprintf(dtc_fplog,"=============================\n");
	fprintf(dtc_fplog,"detcon_lib_th: started %s\n",cptr);
	fprintf(dtc_fplog,"=============================\n");

	detcon_ccd_init_defaults();
//	detcon_ccd_init_config(dtc_fpnull);
	detcon_ccd_init_config(stdout);
	detcon_ccd_init_vars();

	dtc_jpeg1_name[0] = '\0';
	dtc_jpeg2_name[0] = '\0';
	dtc_jpeg1_size[0] = '\0';
	dtc_jpeg2_size[0] = '\0';

	dtc_autosample_temp_per_image = 0;
	if(NULL != (cptr = getenv("CCD_AUTOSAMPLE_TEMP_PER_IMAGE")))
		dtc_autosample_temp_per_image = atoi(cptr);

	return(0);
}

/*
 *	This routine assigns the first round of
 *	values to the "specific..." variables.
 *
 *	The defaults come from this programs
 * 	ccd_dc_defs.h file.  Variables which change
 *	from scanner to scanner or from some
 *	other reason will be altered in the next
 *	phase of initialization.
 */

void	detcon_ccd_init_defaults()
{
	dtc_specific_pixel_size = SPECIFIC_PIXEL_SIZE;
	dtc_specific_outfile_type = SPECIFIC_OUTFILE_TYPE;
	dtc_specific_detector_sn = SPECIFIC_DETECTOR_SN;
	dtc_specific_no_transform = SPECIFIC_NO_TRANSFORM;
	dtc_specific_output_raws = SPECIFIC_OUTPUT_RAWS;
	dtc_specific_j5_trigger = SPECIFIC_J5_TRIGGER;
        dtc_specific_chip_size_x = SPECIFIC_CHIP_SIZE_X;
	dtc_specific_chip_size_y = SPECIFIC_CHIP_SIZE_Y;
	dtc_specific_t2k_detector = SPECIFIC_T2K_DETECTOR;
	dtc_specific_modular = SPECIFIC_MODULAR;
	dtc_specific_gather_return = SPECIFIC_GATHER_RETURN;
}

/*
 *	This routine initializes scanner global
 *	variables now that the program has decided
 *	what the specific parameters actually are.
 */

void	detcon_ccd_init_vars()
{

	dtc_stat_wavelength = 1.0;
	dtc_use_j5_trigger = dtc_specific_j5_trigger;
	dtc_pixel_size = dtc_specific_pixel_size;
        dtc_chip_size_x = dtc_specific_chip_size_x;
        dtc_chip_size_y = dtc_specific_chip_size_y;
	dtc_t2k_detector = dtc_specific_t2k_detector;
	dtc_modular = dtc_specific_modular;
	dtc_check_gather_return = dtc_specific_gather_return;
 
	dtc_stat_xcen = 45.;
	dtc_stat_ycen = 45.;

	dtc_outfile_type = dtc_specific_outfile_type;
	dtc_detector_sn = dtc_specific_detector_sn;
	dtc_output_raws = dtc_specific_output_raws;

	dtc_default_imsize = dtc_chip_size_x * dtc_chip_size_y * 2 * dtc_n_ctrl;

}

/*
 *	This routine handles the name translation
 *	and file opens, leaving all in their proper
 *	state.
 *
 *	Network version:
 *	  Open up the log file.
 *	  Open up the config file.
 *	  Open up the profile file.
 */

int	detcon_ccd_init_files()
{
	char	*cp;

	/*
	 *	Grab some file names from the environment.
	 */

	if(NULL == (cp = getenv(CCD_DC_LOCAL_LOG)))
	{
		fprintf(stderr, "Please set the environment variable: %s\n", CCD_DC_LOCAL_LOG);
		fprintf(stderr, "Then re-execute the program linking to this library.\n");
		detcon_cleanexit(BAD_STATUS);
	}
	else
		strcpy(dtc_lfname, cp);

	if(NULL == (cp = getenv(CCD_DC_CONFIG)))
	{
		fprintf(stderr, "Please set the environment variable: %s\n", CCD_DC_CONFIG);
		fprintf(stderr, "Then re-execute the program linking to this library.\n");
		detcon_cleanexit(BAD_STATUS);
	}
	else
		strcpy(dtc_confname, cp);

	if(NULL == (cp = getenv(CCD_N_CTRL)))
	{
		fprintf(stderr, "\n\nWARNING: environment variable CCD_N_CTRL is NOT set.\n");
		fprintf(stderr, "\tUsing 9 as the number of controllers.\n");
		fprintf(stderr, "\tIf this is incorrect, set the environment CCD_N_CTRL and restart the program.\n");
		dtc_n_ctrl = 9;
	}
	else
		dtc_n_ctrl = atoi(cp);

	/*
	 *	Open up log file only.
	 */
	
	if(NULL == (dtc_fplog = fopen(dtc_lfname,OPENA_REC)))
	{
	    fprintf(stderr,"Cannot open %s as logging file\n",dtc_lfname);
	    detcon_cleanexit(BAD_STATUS);
	}

	return(0);
}

/*
 *	detcon_ccd_init_config
 *
 *	This routine allows the user to override default
 *	values for the scanner specific variables from
 *	a configuration file.
 *
 *	The format of the configuration file is:
 *
 *	keyword		value
 *
 *	The user may specify as little of the formal
 *	keyword as is necessary for unambiguous
 *	determination of the keyword.
 */

/*
 *	keywords:
 */

struct config_key {
			char	*key_name;
			char	*key_abbr;
			int	key_value;
		  };

enum {
	KEY_WAVELENGTH = 0	,
	KEY_PIXEL_SIZE		,
	KEY_OUTFILE_TYPE	,
	KEY_DETECTOR_SN		,
	KEY_NO_TRANSFORM	,
	KEY_OUTPUT_RAWS		,
	KEY_J5_TRIGGER		,
	KEY_CHIP_SIZE_X		,
	KEY_CHIP_SIZE_Y		,
	KEY_T2K_DETECTOR	,
	KEY_MODULAR		,
	KEY_XFORM_WAIT		,
	KEY_GATHER_WAIT
};

struct config_key detcon_config_list[] =
{
		{ "wavelength","wavelength",KEY_WAVELENGTH },
		{ "pixel_size","pixel_size",KEY_PIXEL_SIZE },
		{ "outfile_type","outfile_type",KEY_OUTFILE_TYPE },
		{ "detector_sn","detector_sn",KEY_DETECTOR_SN },
		{ "no_transform","no_transform",KEY_NO_TRANSFORM },
		{ "output_raws","output_raws",KEY_OUTPUT_RAWS },
		{ "j5_trigger","j5_trigger",KEY_J5_TRIGGER },
		{ "t2k_detector", "t2k_detector", KEY_T2K_DETECTOR },
		{ "modular", "modular", KEY_MODULAR },
		{ "xform_wait", "xform_wait", KEY_XFORM_WAIT },
		{ "gather_wait", "gather_wait", KEY_GATHER_WAIT },
		{ "chip_size_x","chip_size_x",KEY_CHIP_SIZE_X },
		{ "chip_size_y","chip_size_y",KEY_CHIP_SIZE_Y },
		{ NULL,NULL,0 }
};

int	detcon_ccd_init_config(FILE *fpmsg)
{
	char	tname[256];
	char	line[132];
	char	string1[132],string2[132];
	int	i,j;

	strcpy(tname,dtc_confname);

	if(NULL == (dtc_fpconfig = fopen(tname,"r")))
	  {
	    fprintf(stderr,"detcon_lib_th: config: cannot open config file %s\n",tname);
	    fprintf(dtc_fplog,"detcon_lib_th: config: cannot open config file %s\n",tname);
	    fflush(dtc_fplog);
	    return(1);
	  }

	while(NULL != fgets(line,sizeof line,dtc_fpconfig))
	{
	    if(line[0] == '!' || line[0] == '#')
	      {
		fprintf(fpmsg,"%s",line);
		fprintf(dtc_fplog,"%s",line);
		continue;
	      }
	    i = sscanf(line,"%s%s",string1,string2);
	    if(i != 2)
	      {
		fprintf(stderr,"detcon_lib_th: config: not enough params (need 2):\n");
		fprintf(stderr,"%s",line);
		fprintf(stderr,"detcon_lib_th: config: ignoring that line.\n");
		continue;
	      }
	    j = 0;
	    for(i = 0; detcon_config_list[i].key_name != NULL; i++)
	      if(0 == strncmp(detcon_config_list[i].key_abbr,string1,strlen(detcon_config_list[i].key_abbr)))
		{
			j = 1;
			break;
		}
	    if(j == 0)
	    {
//		fprintf(stderr,"ccd_dc: config: unrecognized keyword:\n");
//		fprintf(stderr,"%s",line);
//		fprintf(stderr,"ccd_dc: config: ignoring that line.\n");
		continue;
	    }
	    switch(detcon_config_list[i].key_value)
	    {
		case	KEY_DETECTOR_SN:
			sscanf(string2,"%d",&dtc_specific_detector_sn);
			fprintf(fpmsg,"detcon_lib_th: config: %s set to %d\n",
				detcon_config_list[i].key_name,dtc_specific_detector_sn);
			fprintf(dtc_fplog,"detcon_lib_th: config: %s set to %d\n",
				detcon_config_list[i].key_name,dtc_specific_detector_sn);
			break;
		case	KEY_PIXEL_SIZE:
			sscanf(string2,"%f",&dtc_specific_pixel_size);
			fprintf(fpmsg,"detcon_lib_th: config: %s set to %.6f\n",
				detcon_config_list[i].key_name,dtc_specific_pixel_size);
			fprintf(dtc_fplog,"detcon_lib_th: config: %s set to %.6f\n",
				detcon_config_list[i].key_name,dtc_specific_pixel_size);
			break;
		case	KEY_NO_TRANSFORM:
			sscanf(string2,"%d",&dtc_specific_no_transform);
			fprintf(fpmsg,"detcon_lib_th: config: %s set to %d\n",
				detcon_config_list[i].key_name,dtc_specific_no_transform);
			fprintf(dtc_fplog,"detcon_lib_th: config: %s set to %d\n",
				detcon_config_list[i].key_name,dtc_specific_no_transform);
			break;
		case	KEY_OUTPUT_RAWS:
			sscanf(string2,"%d",&dtc_specific_output_raws);
			fprintf(fpmsg,"detcon_lib_th: config: %s set to %d\n",
				detcon_config_list[i].key_name,dtc_specific_output_raws);
			fprintf(dtc_fplog,"detcon_lib_th: config: %s set to %d\n",
				detcon_config_list[i].key_name,dtc_specific_output_raws);
			break;
		case	KEY_J5_TRIGGER:
			sscanf(string2,"%d",&dtc_specific_j5_trigger);
			fprintf(fpmsg,"detcon_lib_th: config: %s set to %d\n",
				detcon_config_list[i].key_name,dtc_specific_j5_trigger);
			fprintf(dtc_fplog,"detcon_lib_th: config: %s set to %d\n",
				detcon_config_list[i].key_name,dtc_specific_j5_trigger);
			break;
		case	KEY_OUTFILE_TYPE:
			if(0 == strcmp(string2,"signed_long") ||
			   0 == strcmp(string2,"int"))
				dtc_outfile_type = 1;
			if(dtc_outfile_type == 0)
			  {
			    fprintf(fpmsg,"detcon_lib_th: config: %s set to unsigned_short\n",
				detcon_config_list[i].key_name);
			    fprintf(dtc_fplog,"detcon_lib_th: config: %s set to unsigned_short\n",
				detcon_config_list[i].key_name);
			  }
			 else
			  {
			    fprintf(fpmsg,"detcon_lib_th: config: %s set to signed_long\n",
				detcon_config_list[i].key_name);
			    fprintf(dtc_fplog,"detcon_lib_th: config: %s set to signed_long\n",
				detcon_config_list[i].key_name);
			  }
			break;
                case    KEY_CHIP_SIZE_X:
                        sscanf(string2,"%d",&dtc_specific_chip_size_x);
                        fprintf(fpmsg,"detcon_lib_th config: %s set to %d\n",
                                detcon_config_list[i].key_name,dtc_specific_chip_size_x);
                        fprintf(dtc_fplog,"detcon_lib_th config: %s set to %d\n",
                                detcon_config_list[i].key_name,dtc_specific_chip_size_x);
                        break;
                case    KEY_CHIP_SIZE_Y:
                        sscanf(string2,"%d",&dtc_specific_chip_size_y);
                        fprintf(fpmsg,"detcon_lib_th config: %s set to %d\n",
                                detcon_config_list[i].key_name,dtc_specific_chip_size_y);
                        fprintf(dtc_fplog,"detcon_lib_th config: %s set to %d\n",
                                detcon_config_list[i].key_name,dtc_specific_chip_size_y);
                        break;
                case    KEY_MODULAR:
                        sscanf(string2,"%d",&dtc_specific_modular);
                        fprintf(fpmsg,"detcon_lib_th config: %s set to %d\n",
                                detcon_config_list[i].key_name,dtc_specific_modular);
                        fprintf(dtc_fplog,"detcon_lib_th config: %s set to %d\n",
                                detcon_config_list[i].key_name,dtc_specific_modular);
                        break;
                case    KEY_XFORM_WAIT:
                case    KEY_GATHER_WAIT:
                        sscanf(string2,"%d",&dtc_specific_gather_return);
                        fprintf(fpmsg,"detcon_lib_th config: %s set to %d\n",
                                detcon_config_list[i].key_name,dtc_specific_gather_return);
                        fprintf(dtc_fplog,"detcon_lib_th config: %s set to %d\n",
                                detcon_config_list[i].key_name,dtc_specific_gather_return);
                        break;
	    }
	}
	fflush(dtc_fplog);
	fclose(dtc_fpconfig);

	return(0);
}
