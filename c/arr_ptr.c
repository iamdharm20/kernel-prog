#include <stdio.h>

int main()
{
	int i = 0;
	int arr[3] = {1,2,3};

	/* Note: arr name repersent a int * and &arr repersent a int (*)[n]
	* arr + 1 => next element address and &arr + 1 => next array address
	* arr is not a simple variable, its a constant pointer
	* arr++ is not possible. 
	*/
	int *n = arr;
	int *n1 = &arr; /* warning: initialization from incompatible pointer */
	for (i = 0; i < 3; i++) {
		printf("arr[%d]:%d : %p\n", i, *n, n);
		printf("arr[%d]:%d : %p\n", i, *n1, n1);
		printf("arr[%d]:%d : %p\n", i, *(arr + i), arr + i);
		n++;	/* incrment 4 byte as data type is integer */
		n1++;	/* incrment 4 byte as data type is integer */
	}
	printf("\n");

	/* Points to 0th element of the arr. 
	 * p = arr => p = &arr[0]
	 * arr[i] == *(arr + i)
	*/
	int (*p)[3] = arr; /* warning: initialization from incompatible pointer */
	for (i = 0; i < 3; i++) {
		printf("arr[%d]:%d : %p\n", i, *p, p);	/* *p == arr => *&arr => arr i.e * and & will cancel each oter*/

		p++;	/* incrment by whole array means 12 byte as data type is int (*)[3] */
	}
	printf("\n");

	#if 0
	p + 0 = 20; 	/* error: lvalue required as left operand of assignment */
	p[0] = 20;	/* assignment to expression with array type */
	arr++;		/* error: lvalue required as increment operand */
	arr = arr + 1;	/* error: assignment to expression with array type */
	#endif

	/* Points to the whole array arr */
	int (*ptr)[3] = &arr;

	printf("%p %p \n",&arr[0], ptr[0]);

	printf("p = %p, ptr = %p\n", p, ptr);
	printf("*p = %d, *ptr = %p\n", *p, *ptr);
	printf("sizeof(ptr) = %lu, sizeof(*ptr) = %lu\n\n", sizeof(ptr), sizeof(*ptr));

	/* Note: *(*ptr + i) => *(*&arr + i) => *(arr + i) => arr[i] */	
	for (i = 0; i < 3; i++) {
		printf("arr[%d]:%d\n", i, *(*ptr + i));
	}

	
	return 0;
}
