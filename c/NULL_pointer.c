#include <stdio.h>
#include <stdlib.h>
/*NULL Pointer is a pointer which is pointing to nothing. 
Most common use cases for NULL are
a) To initialize a pointer variable when that pointer variable isn’t assigned any valid memory address yet.
b) To check for null pointer before accessing any pointer variable. By doing so, we can perform error handling in pointer related code 
	e.g. dereference pointer variable only if it’s not NULL.
c) To pass a null pointer to a function argument when we don’t want to pass any valid memory address.

steps while using pointers:
1. Always initialize pointer variables as NULL.
2. Always perform NULL check before accessing any pointer.

What is the difference between NULL and Void Pointer?
	The void is one of the generic data type which can be typecast in any datatype in C. 
	Whereas, NULL is the value which is assigned to the pointer.

void pointer :
	void pointers cannot be dereferenced. It can however be done using typecasting the void pointer
	Pointer arithmetic is not possible on pointers of void due to lack of concrete value and thus size.
Dangling pointer:
	A pointer pointing to a memory location that has been deleted (or freed) is called dangling pointer.
*/
int main()
{

	int *ptr1 = NULL;
	/*If ptr is NULL, no operation is performed.*/
	free(ptr1);
	

	/* Can we use sizeof() operator on NULL in C? Well, usage of sizeof(NULL) is allowed 
	but the exact size would depend on platform.*/
	printf("sizeof(NULL): %ld\n", sizeof(NULL)); /* sizeof NULL Macro is 8 byte in 64bit system as NULL is a void pointer ((void*)0) */
	printf("sizeof(void): %ld\n",sizeof(void)); /* why sizeof(void) is one in gcc */
	printf("NULL: %d\n", NULL);
	/* What about dereferencing of NULL? compile successfully but crashes/segmentation fault when the program is runs */
	printf("NULL pointer dereferance: %d\n",*ptr1);

	int *ptr = malloc(sizeof(int));
	free(ptr); /* ptr pointer is pointing to a dangling reference so always make it NULL like ptr = NULL */
	ptr = NULL; /* now ptr is not a dangling pointer */	
	/*if  free(ptr)  has  already been called before, undefined behavior occurs.*/
	free(ptr); /* Error: double free or corruption (fasttop): 0x0000000002383010 */

	int *ii = NULL, *jj = NULL;
	if(ii == jj)
		printf("This is always printed coz ii and jj are same\n");

		
	return 0;
}
