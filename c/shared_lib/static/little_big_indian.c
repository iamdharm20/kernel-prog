#include <stdio.h>

void check_indianness(char *);
void little_to_big_indian(int *nptr);

/*Main function to call above function for 0x01234567*/
int main()
{
	int var = 0x12345678;
	int ret = 0;
	
	char *ch = (char *)&var;

	int *nptr = &var;
	
	check_indianness(ch);
	little_to_big_indian(nptr);

	return 0;
}
