#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<unistd.h>
#include	"../incl/detcon_par.h"
#include	"../incl/detcon_state.h"

static	int	bin;
static	int	adc;
static	int	output_raws;
static	int	no_transform;
static	int	flp_kind;
static	float	beam_xcen;
static	float	beam_ycen;
static	int	compress;
static	int	dist;
static	int	twotheta;
static	int	axis;
static	float	wave;
static	char	infobuf[256];
static	float	stat_time;
static	float	osc_width;
static	float	phi;
static	int	stored_dark;
static	float	start_time;
static	float	inc_time;

int     CCDStartExposure();
int     CCDStopExposure();
char    *CCDStatus();
int     CCDState();
int     CCDSetFilePar(int which_par, void *p_value);
int     CCDGetFilePar(int which_par,void *p_value);
int     CCDSetHwPar(int which_par,void *p_value);
int     CCDGetHwPar(int which_par, void *p_value);
int     CCDSetBin(int val);
int     CCDGetBin();
int     CCDGetImage();
int     CCDCorrectImage();
int     CCDWriteImage();
char    *CCDGetLastError();
int     CCDAbort();
int     CCDReset();
int     CCDInitialize();
int     CCD_HWReset();
int     CCDSetProperty(char *property, const char *value, int overwrite);


void	usage(FILE *fp)
{
	fprintf(stderr,"detcon_th_mult_seq [-sd] [-xf] [-bin (for testing ccds)] init_time prefix start ngroups nrep time_inc\n");
}

int	takeimages(char *prefix, int start_frame, int num_images, int image_kind)
{
	char	buf[256];
	int	last_image;
	int	order = 1;

	sprintf(buf,"%s_%03d", prefix, 1);
	flp_kind = image_kind;

	CCDSetFilePar(FLP_TIME,&stat_time);
	CCDSetFilePar(FLP_FILENAME,buf);
	CCDSetFilePar(FLP_KIND,&flp_kind);

	// CCDSetHwPar(HWP_MULT_FRAME_START, &start);
	CCDSetHwPar(HWP_MULT_NFRAME, &num_images);
	    
	CCDStartExposure();
	while(DTC_STATE_EXPOSING != CCDState())
	{
		if(DTC_STATE_ERROR == CCDState())
		{
			fprintf(stdout,"Error returned from CCDStartExposure()\n");
			return(1);
		}
	}
	
	if(order == 0)
	{
		last_image = 1;
		CCDSetFilePar(FLP_LASTIMAGE, &last_image);
		CCDGetImage();
	}
	fprintf(stdout,"sleeping for %.4f seconds, simulating %d exposures\n", num_images * stat_time, num_images);
	

	usleep((long) (num_images * stat_time * 1000000));

	CCDStopExposure();
	while(DTC_STATE_IDLE != CCDState())
	{
		if(DTC_STATE_ERROR == CCDState())
		{
			fprintf(stdout,"Error returned from CCDStartExposure()\n");
			return(1);
		}
	}
	if(order == 1)
	{
		last_image = 1;
		CCDSetFilePar(FLP_LASTIMAGE, &last_image);
		CCDGetImage();
	}
	return(0);
}

int	main(int argc, char *argv[])
{
	char	prefix[512];
	char	sent_prefix[512];
	int	start, dez;
	int	m, nrep, ngroups;
	int	res;
	int	probe_port_raw(int fd);

	bin = 1;
	adc = 1;
	output_raws = 1;
	no_transform = 1;
	flp_kind = 0;
	beam_xcen = 90;
	beam_ycen = 90;
	compress = 0;
	dist = 40;
	twotheta = 0;
	axis = 1;
	wave = 1.5418;
	stat_time = 30;
	osc_width = 1.0;
	phi = 0.0;
	dez = 0;
	stored_dark = 0;

	while(argc > 1 && '-' == argv[1][0])
	{
		if(0 == strcmp("-sd", argv[1]))
		{
			stored_dark = 1;
			argv++;
			argc--;
		}
		if(0 == strcmp("-xf", argv[1]))
		{
			no_transform = 0;
			argv++;
			argc--;
		}
		if(0 == strcmp("-bin", argv[1]))
		{
			bin = 2;
			argv++;
			argc--;
		}
	}
	if(argc < 7)
	{
		usage(stderr);
		exit(0);
	}

	start_time = atof(argv[1]);
	strcpy(prefix, argv[2]);
	start = atoi(argv[3]);
	ngroups = atoi(argv[4]);
	nrep = atoi(argv[5]);
	inc_time = atof(argv[6]);

	CCDInitialize();

	// system("sleep 5");	/* immune to interrupt bounce outs */

	while(DTC_STATE_IDLE  != CCDState())
	{
		fprintf(stdout,"%d\n",CCDState());
	}

	/*
 	 *	These parameters define the state of the CCD's hardware and the
	 *	disposition of output images, via the transform.
	 */

        CCDSetHwPar(HWP_BIN, &bin);
        CCDSetHwPar(HWP_ADC, &adc);
        CCDSetHwPar(HWP_SAVE_RAW, &output_raws);
        CCDSetHwPar(HWP_NO_XFORM, &no_transform);
        CCDSetHwPar(HWP_STORED_DARK, &stored_dark);

	/*
 	 *	Nice to have these paramters set for the header of the image.
	 */

	for(m = start ; m < (start + ngroups) ; m++)
	{
		sprintf(sent_prefix, "%s_%03d", prefix, m);

		stat_time = start_time + (m - 1) * inc_time;

		CCDSetFilePar(FLP_KIND,&flp_kind);
		CCDSetFilePar(FLP_FILENAME,infobuf);
		CCDSetFilePar(FLP_TIME,&stat_time);
        	CCDSetFilePar(FLP_BEAM_X, &beam_xcen);
        	CCDSetFilePar(FLP_BEAM_Y, &beam_ycen);
        	CCDSetFilePar(FLP_COMPRESS, &compress);
        	CCDSetFilePar(FLP_COMMENT,NULL);
        	CCDSetFilePar(FLP_DISTANCE,&dist);
        	CCDSetFilePar(FLP_TWOTHETA,&twotheta);
        	CCDSetFilePar(FLP_AXIS,&axis);
        	CCDSetFilePar(FLP_WAVELENGTH,&wave);
        	CCDSetFilePar(FLP_OSC_RANGE,&osc_width);
		CCDSetFilePar(FLP_PHI,&phi);

		if(0 == stored_dark)
		{
			if(takeimages(sent_prefix, m, 1, 0))
				exit(0);

			if(takeimages(sent_prefix, m, 1, 1))
				exit(0);
		}

		if(takeimages(sent_prefix, m, nrep, 5))
			exit(0);
		res = probe_port_raw(0);
		if(1 == res)
		{
			char	line[120];
			fgets(line, sizeof line, stdin);
			fprintf(stdout, "Enter x to exit, anything else to resume:");
			fgets(line, sizeof line, stdin);
			if('x' == line[0])
			{
				sleep(1);
				break;
			}
		}
	}

	fprintf(stdout,"Done.\n");
	fprintf(stdout,"Wait 2 seconds for all images to be collected by gather\n");
	system("sleep 2");
	fprintf(stdout,"Exiting normally\n");
	exit(0);
}
