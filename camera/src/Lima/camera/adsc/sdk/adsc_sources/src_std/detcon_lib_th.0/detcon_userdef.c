#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>

int	userdef_module_return(int mn, char *to, char *userdef_returned)
{
	char	*urb, *cp_mn, *cp_mn1, *cp_msg, *cp_msg1, *cp;
	char	buf[80];
	int	n;
	char	*mn_str = "__module_number__";
	int	mn_strlen;
	char	*msg_str = "__command_msg__";
	int	msg_strlen;
	char	*userret_str = "userret\n";
	int	userret_strlen;
	int	i;

	mn_strlen = strlen(mn_str);
	msg_strlen = strlen(msg_str);
	userret_strlen = strlen(userret_str);

	urb = userdef_returned;

	while(1)
	{
		if(NULL == (cp_mn = strstr(urb, mn_str)))
			break;
		cp_mn1 = strstr(cp_mn + mn_strlen, mn_str);
		for(i = 0, cp = cp_mn + mn_strlen; cp < cp_mn1; cp++, i++)
			buf[i] = *cp++;
		buf[i + 1] = '\0';

		n = atoi(buf);
		
		urb = cp_mn1 + mn_strlen;

		if(mn != n)
			continue;

		cp_msg = strstr(urb, msg_str) + msg_strlen;
		cp_msg1 = strstr(cp_msg, msg_str);
		cp_msg += userret_strlen;
		
		for(cp = cp_msg; cp < cp_msg1; )
			*to++ = *cp++;
		*to = '\0';
		return(1);
	}

	return(0);
}


