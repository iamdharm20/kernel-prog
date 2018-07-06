#include <stdio.h>
#include "callback.h"
 
/* registration goes here */
void register_callback(struct math_operation *ptr_reg_callback)
{
	int sum = 0, num1 = 20, num2 = 10;

	printf("inside: %s\n", __func__);

	/* calling our callback function my_callback */
	printf("calling our callback function my_add_callback\n");
	sum = (ptr_reg_callback->add)(num1, num2);
	printf("sum = %d\n", sum);
}
