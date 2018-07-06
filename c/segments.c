#include <stdio.h>

/* use objdump, size, free, gdb -tui a.out for debugging */

const int num = 10;	//.rodata
int data1 = 10;		// .data

void func1(char *ptr, int **ptr1)
{
	int data = 17;
	
	*ptr1 = &data;	/* data overwritten properly why and how ptr1 can hold the local stack address */
}

void func(char *ptr, int *ptr1)
{
	const int num;		/* stack segment : read only memory*/
	num = 13;		/* error: assignment of read-only variable ‘num’ as const nummust be intilized during declartaion*/

	int data = 15;

	*ptr1 = data;		/* data overwritten properly */

	ptr1 = &data;
	*ptr1 = 17;		/*Here data is not overwritten why? hint: pass the pointer to a pinter 
				eg. int **ptr1 and func(ptr, &ptr1) */
}

int main()
{
	char *ptr = "sharma";	//.rodata

	static int data2;	//.bss

	int num2 = 12;
	int *ptr1 = &num2;
	
	func(ptr, ptr1);
	printf("*ptr1:%d\n", *ptr1);
	
	func1(ptr, &ptr1);
	printf("*ptr1:%d\n", *ptr1);
	
	return 0;
}
