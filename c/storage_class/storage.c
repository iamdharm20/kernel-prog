#include <stdio.h>

/*how to access static function in other file: by using function pointer*/
extern int (* fptr) (int (*)[], int (*)[], int (*)[]);

int main()
{

	int a[6] = {1,5,7,8,9,10};
	int b[5] = {2,3,4,11,12};
	int c[11] ={0};
	
	int ret = (*fptr)(&a, &b, &c);
	printf("ret = %d\n", ret);

	return 0;
}
