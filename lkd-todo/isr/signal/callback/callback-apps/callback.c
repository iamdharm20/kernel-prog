/* callback.c */
#include<stdio.h>
#include"reg_callback.h"
 
/* callback function definition goes here */
void my_callback(void)
{
	printf("inside my_callback\n");
}
 
int main(void)
{
	/* initialize function pointer to my_callback */
	callback = my_callback;                           
	printf("This is a program demonstrating function callback\n");
	
	/* register our callback function */
	register_callback(callback);                             
	printf("back inside main program\n");
	
	return 0;
}
