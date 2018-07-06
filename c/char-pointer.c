#include <stdio.h>
#include <string.h>

char arr[10];

int main()
{
	static char *msg = arr;

	memset(msg, 0, 10);

	if(msg != NULL) {	
		msg[0] = 'h';
		msg[1] = 'e';
		msg[2] = 'l';
		msg[3] = 'l';
		msg[4] = 'o';
		msg[5] = '\0';
	}
	printf("msg : %s\n", msg);

	return 0;
}
