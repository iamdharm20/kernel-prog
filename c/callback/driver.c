#include <stdio.h>
#include "callback.h"

/* callback functions definition goes here */
int my_add_callback(int num1, int num2) {

	printf("inside : %s defination\n", __func__);
	return num1 + num2;
}

int my_sub_callback(int num1, int num2) {
	
	printf("inside : %s defination\n", __func__);
	return num1 - num2;
}

int my_mull_callback(int num1, int num2) {
	
	printf("inside : %s defination\n", __func__);
	return num1 * num2;
}

int main() {

	/* initialize function pointer's to my_callbacks */
	struct math_operation mops = {
		.add = my_add_callback,
		.sub = my_sub_callback,
		.mul = my_mull_callback,
	};

	printf("This is a program demonstrating function callback\n");

	/* register our callback function */
	register_callback(&mops);

	printf("back inside main program\n");

	return 0;
}
