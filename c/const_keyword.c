/*Advantage of const over Macro:
compiler do not check the macro types its just replace the Macro with vale.
which increase the size of executable.
	while compiler check the constant type.
constant can be defined over a scope. it store the value in memory.
	while macro has file scope.

"const" is just a data QUALIFIER, which means that first the compiler has to decide which segment the variable has to be stored 
and then if the variable is a const, then it qualifies to be stored in the write protected region of that particular segment
*/

#include <stdio.h>
void func( int * ptr, int *ptr1, int const * ptrToConst, int * const constPtr, int const * const constPtrToConst )
{
	*ptr = 0; // OK: modifies the "pointee" data
	ptr  = NULL; // OK: modifies the pointer

	*ptr1 = 30; // OK: modifies the "pointee" data
	ptr1 = NULL; // OK: modifies the pointer
	
	*ptrToConst = 0; // Error! error: assignment of read-only location ‘*ptrToConst’ 
	ptrToConst  = NULL; // OK: modifies the pointer

	*constPtr = 0; // OK: modifies the "pointee" data
	constPtr  = NULL; //error: assignment of read-only parameter ‘constPtr’

	*constPtrToConst = 0; // error: assignment of read-only location ‘*constPtrToConst’
	constPtrToConst  = NULL; // error: assignment of read-only parameter ‘constPtrToConst’
}

int main()
{

	int num = 10;
	int const *ptr = &num;
	num = 20;	// valid as num is not a constant variable
	*ptr = 20;	//error: assignment of read-only location ‘*ptr’
	
	const int num1 = 20;
	num1 = 30; 	//error: assignment of read-only variable ‘num1’
	num1++;		//error: assignment of read-only variable ‘num1’
	int const * ptrToConst = &num1;
	*ptrToConst = 25;	//error: assignment of read-only location ‘*ptrToConst’	

	int * const constPtr = &num;
	int const * const constPtrToConst = &num1;

	func(&num, &num1, ptrToConst, constPtr, constPtrToConst);

	printf("num1: %d\n", num1);
	
	return 0;
}
