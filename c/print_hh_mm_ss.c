#include <stdio.h>

int main()
{	
	int hh, mm, sec;

	printf("Enter number in sec:");
	scanf("%d", &sec);

	hh = sec/3600;
	sec = sec % 3600;

	mm = sec/60;
	sec = sec % 60;

	printf("%d:%d:%d\n", hh, mm, sec);
	return 0;
}
