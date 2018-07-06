#include <stdio.h>
/*Default initial value of static integral type variables are zero otherwise null.
A same static variable can be declared many times but we can initialize at only one time.
We cannot write any assignment statement globally.
A static variable initializes only one time in whole program.
If we declared static variable locally then its visibility will within a block where it has declared.
If declared a static variable or function globally then its visibility will only the file in which it has declared not in the other files.
*/

static char ch;
static char *ptr;
static int i = 10;

int main()
{
	int j = 0;

	printf("%d : %d : %p\n", ch, i, ptr);	
	
	for (j = 0; j < 5; j++) {

		static int i = 20; /* This statment will execute only one time */
		printf("%d\n", i++); /* this statement will execute every time*/
	
	}

	printf("%d\n", i);  /*variable i is not visible here.*/

	return 0;
}
