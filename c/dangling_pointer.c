// The pointer pointing to local variable becomes
// dangling when local variable is static.
#include<stdio.h>
 
int *fun()
{
	/* int x = 5; will cause Segmentation fault while derefrancing in main function why?*/
 	/* x is local variable and goes out of scope after an execution of fun() is over. */ 
	
	static int x = 5; /* valid as static store in data segment */
	return &x;
}
 
int main()
{
	int *p = fun();

	// p points to something which is not valid anymore in case of non static variable so it will be dangling pointer.
	printf("%d", *p);
	return 0;
}
