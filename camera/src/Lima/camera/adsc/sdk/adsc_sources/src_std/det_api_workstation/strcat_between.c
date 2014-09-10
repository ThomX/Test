#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>

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

main(int argc, char *argv[])
{
	char	output[10240];

	if(argc != 5)
	{
		fprintf(stderr,"strcat_between to from between_this and_that\n");
		exit(0);
	}

	strcpy(output, argv[1]);
	strcat_between(output, argv[2], argv[3], argv[4]);
	fprintf(stdout,"%s\n", output);
}
