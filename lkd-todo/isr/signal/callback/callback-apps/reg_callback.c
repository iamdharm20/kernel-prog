/* reg_callback.c */
#include<stdio.h>
#include"reg_callback.h"
 
/* registration goes here */
void register_callback( void (*ptr_reg_callback)(void))
{
	printf("inside register_callback\n");
	/* calling our callback function my_callback */
	(*ptr_reg_callback)();                                  
}
