#include <stdio.h>
//#include "extern.h"

/*Declaration: of a variable/function simply declares that the variable/function exists somewhere in the program 
	but the memory is not allocated for them.
Defination: when we define a variable/function, apart from the role of declaration, it also allocates memory for that variable/function. 
Assignment: Assigning any value to the variable at the time of declaration is known as initialization while assigning any value 
	to variable not at the time of declaration is known assignment.
Program: it may be simple file or group of multiple files.
Storage class: is modifier or qualifier of data types which decides:
	1.  In which area of memory a particular variable will be stored?  
	2. What is scope of variable?
	3. What is visibility of variable?
Visibility: Visibility means accessibility. Up to witch part or area of a program, we can access a variable, 
	that area or part is known as visibility of that variable. 
Scope: Meaning of scope is to check either variable is alive or dead. Alive means data of a variable has not destroyed from memory. 
	Up to which part or area of the program a variable is alive, that area or part is known as scope of a variable.

Note: If any variable is not visible it may have scope i.e. it is alive or may not have scope. 
	But if any variable has not scope i.e. it is dead then variable must not to be visible.

& is the reference operator. Referencing means taking the address of an existing variable (using &) to set a pointer variable.
* is the dereference operator. Dereferencing a pointer means using the * operator (asterisk character) to access the value stored at a pointer.

Declaration can be done any number of times but definition only once. 
When extern is used with a variable, it’s only declared not defined.
If we declare any variable as extern variable then it searches that variable either it has been initialized or not. 
	If it has been initialized which may be either extern or static* then it is ok otherwise compiler will show an error.
A particular extern variable can be declared many times but we can initialize at only one time.
As an exception, when an extern variable is declared with initialization, it is taken as definition of the variable as well.
extern is a default storage class of all global variables as well all functions.
We cannot initialize extern variable locally i.e. within any block either at the time of declaration or separately. 
	We can only initialize extern variable globally.
We cannot write any assignment statement globally.
If declared an extern variables or function globally then its visibility will whole the program which may contain one file or many files.
*/


extern int var; /* declaration of var variable */
extern int data = 10; /* declaration + defination of data variable */

int main()
{
	printf("%d\n", var); 	/* Error: undefined reference to `var'*/
	printf("%d\n", data);	/* warning: ‘data’ initialized and declared ‘extern’ */

	{
		extern int i = 10; /* error: ‘i’ has both ‘extern’ and initializer so We cannot initialize extern variable locally */
		printf("%d\n", i);
	}

	{
		extern int i;
		int i = 10;	/* error: declaration of ‘i’ with no linkage follows extern declaration */
		printf("%d\n", i);
	}
	return 0;
}
