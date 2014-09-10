#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	"../incl/detcon_par.h"
#include	"../incl/detcon_state.h"

/*
 *	Wait for either IDLE or ERROR state, using 100 msec sleeps between
 *	checks so as not to overload the CPU with status checks.
 *
 *	Return 0 if the state detected was IDLE, or 1 if ERROR.
 */

int	wait_for_idle_or_error()
{
	int	state;

	while(1)
	{
		state = CCDState();
		if(DTC_STATE_IDLE == state)
			return(0);
		if(DTC_STATE_ERROR == state)
			return(1);
		usleep(100000);
	}
	return(1);
}

float	temp_read[9], temp_target[9], temp_final[9], temp_inc[9], fval;

test_print_temp_readings(char *readings)
{
	char	checkbuf[128];
	int	detret;
	int	n_ctrl, n, done, stat, cold;
	int	temp_status[9];
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
}

void	usage(FILE *fp)
{
	fprintf(fp, "\nUsage:  detcon_temp_cmd_th <command> (see below)\n\n");
	fprintf(fp, "\tdetcon_temp_cmd_th read_temps [<n_reads>]\n");
	fprintf(fp, "\tdetcon_temp_cmd_th set_temp to_value\n");
	fprintf(fp, "\tdetcon_temp_cmd_th ramp_temp to_value\n");
	fprintf(fp, "\tdetcon_temp_cmd_th make_cold\n");
	fprintf(fp, "\tdetcon_temp_cmd_th make_warm\n");
	fprintf(fp, "\tdetcon_temp_cmd_th is_cold\n");
	fprintf(fp, "\tdetcon_temp_cmd_th stable_reading\n");
	fprintf(fp, "\tdetcon_temp_cmd_th abort\n");
	fprintf(fp, "\tdetcon_temp_cmd_th status\n");
	fprintf(fp, "\n");
}

char	*cmds[] = {
			"read_temps",
			"set_temp",
			"ramp_temp",
			"make_cold",
			"make_warm",
			"is_cold",
			"is_warm",
			"stable_reading",
			"abort",
			"status",
			NULL
		  };

enum	cmd_nos {
			CMD_READ_TEMPS = 0,
			CMD_SET_TEMP,
			CMD_RAMP_TEMP,
			CMD_MAKE_COLD,
			CMD_MAKE_WARM,
			CMD_IS_COLD,
			CMD_IS_WARM,
			CMD_STABLE_READING,
			CMD_ABORT,
			CMD_STATUS,
			BAD_CMD
		};

main(argc,argv)
int	argc;
char	*argv[];
  {
	int	i, n, n_readings;
	float	set_val, ramp_val, temp_val;
	char	temp_readings[1024];
	int	temp_cold;
	int	stat;
	int	cold_state;
	int	warm_state;
	char	line[100];
	char	*cp;
	int	temp_mode;

	if(argc < 2)
	{
		usage(stdout);
		exit(0);
	}

	for(n = 0; cmds[n] != NULL; n++)
		if(0 == strcmp(cmds[n], argv[1]))
			break;

	if(BAD_CMD == n)
	{
		fprintf(stderr,"detcon_temp_cmd: %s is an unrecognized command\n", argv[1]);
		usage(stdout);
		exit(0);
	}

	switch(n)
	{
		case CMD_READ_TEMPS:
			if(argc < 3)
				n_readings = 1;
			else
				n_readings = atoi(argv[2]);
			for(i = 0; i < n_readings; i++)
       				CCDGetHwPar(HWP_TEMP_VALUE, &temp_val);
       			CCDGetHwPar(HWP_TEMP_STATUS, &temp_readings[0]);
			fprintf(stderr, "read_temp: temp_readings is:\n%s\n", temp_readings);
			test_print_temp_readings(temp_readings);
			break;

		case CMD_SET_TEMP:
			if(argc < 3)
			{
				fprintf(stderr,"detcon_temp_cmd set_temp needs an argument\n");
				break;
			}
			sscanf(argv[2], "%f", &set_val);
			if(set_val < -47. || set_val > 20.)
			{
				fprintf(stderr,"detcon_temp_cmd:  Invalid value (%8.2f) for set temperature.\n", set_val);
				fprintf(stderr,"detcon_temp_cmd:  Value must be:  -45. < value < 20\n");
				break;
			}
			temp_mode = 1;		/* for SET Temp as opposed to RAMP Temp */
       			CCDSetHwPar(HWP_TEMP_MODE, &temp_mode);
       			CCDSetHwPar(HWP_TEMP_VALUE, &set_val);
			
			wait_for_idle_or_error();

			break;

		case CMD_RAMP_TEMP:
			if(argc < 3)
			{
				fprintf(stderr,"detcon_temp_cmd ramp_temp needs an argument\n");
				break;
			}
			sscanf(argv[2], "%f", &ramp_val);
			if(set_val < -47. || set_val > 20.)
			{
				fprintf(stderr,"detcon_temp_cmd:  Invalid value (%8.2f) for ramp temperature.\n", ramp_val);
				fprintf(stderr,"detcon_temp_cmd:  Value must be:  -45. < value < 20\n");
				break;
			}

			temp_mode = 0;		/* for RAMP Temp as opposed to SET Temp */
       			CCDSetHwPar(HWP_TEMP_MODE, &temp_mode);
       			CCDSetHwPar(HWP_TEMP_VALUE, &ramp_val);
			
			wait_for_idle_or_error();
			
			break;

		case CMD_MAKE_COLD:

			cold_state = 1;
       			CCDSetHwPar(HWP_TEMP_COLD, &cold_state);

			wait_for_idle_or_error();

			break;

		case CMD_MAKE_WARM:

			warm_state = 1;
       			CCDSetHwPar(HWP_TEMP_WARM, &warm_state);

			wait_for_idle_or_error();

			break;

		case CMD_IS_COLD:

       			CCDGetHwPar(HWP_TEMP_VALUE, &temp_val);
       			CCDGetHwPar(HWP_TEMP_STATUS, &temp_readings[0]);
       			CCDGetHwPar(HWP_TEMP_COLD, &cold_state);

			wait_for_idle_or_error();

			fprintf(stdout,"%d\n", cold_state);

			break;

		case CMD_IS_WARM:

       			CCDGetHwPar(HWP_TEMP_VALUE, &temp_val);
       			CCDGetHwPar(HWP_TEMP_STATUS, &temp_readings[0]);
       			CCDGetHwPar(HWP_TEMP_WARM, &warm_state);

			wait_for_idle_or_error();

			fprintf(stdout,"%d\n", warm_state);

			break;

		case CMD_STABLE_READING:
			for(i = 0; i < 5; i++)
       				CCDGetHwPar(HWP_TEMP_VALUE, &temp_val);
       			CCDGetHwPar(HWP_TEMP_STATUS, &temp_readings[0]);
			test_print_temp_readings(temp_readings);
			break;

		case CMD_ABORT:
       			CCDAbort();

			wait_for_idle_or_error();

			break;

		case CMD_STATUS:
       			CCDGetHwPar(HWP_TEMP_STATUS, &temp_readings[0]);
			test_print_temp_readings(temp_readings);

			break;

		default:
			fprintf(stderr,"detcon_temp_cmd:  ERROR: should never get here.\n");
	}

	exit(0);
}
