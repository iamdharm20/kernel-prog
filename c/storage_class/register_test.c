#include <stdio.h>

/*A register storage class is very similar to auto storage class except one most important property. 
	All register variable in c stores in CPU not in the memory.
A register variable execute faster than other variables because it is stored in CPU so during the execution compiler 
	has no extra burden to bring the variable from memory to CPU.
Since a CPU have limited number of register so it is programmer responsibility which variable should declared 
	as register variable i.e. variable which are using many times should declared as a register variable.
We cannot dereference register variable since it has not any memory address.
Default initial value of register variable is garbage.
Scope and visibility of register variable is block.
*/
int main()
{
	register int i = 10;
	
	int *ptr = &i; /* error: address of register variable ‘i’ requested */

	printf("%d : %p : %d : %p\n", i, &i, *ptr, ptr);
	return 0;
}
