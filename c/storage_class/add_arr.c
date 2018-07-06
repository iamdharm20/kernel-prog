#include <stdio.h>

static int add_shorted_arr(int (*p1)[], int (*p2)[], int (*p3)[])
{
	printf("%s\n", __func__);
	return 0;
}

int (* fptr) (int (*)[], int (*)[], int (*)[]) = add_shorted_arr;
