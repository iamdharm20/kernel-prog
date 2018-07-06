#include <stdio.h>

int fun1(int a, int b) {
	return a + b;
}

int fun2(int a, int b) {
	return a - b;
}

int fun3(int a, int b) {
	return a * b;
}

int main()
{
	int i = 0;
	int a = 20, b = 10;
	
	int (*arr[4])(int, int) = {&fun1, &fun2, &fun3};
	
	int (*(*ptr)[3])(int, int) = arr;

	for(i = 0; i <3; i++) {
		printf("%d\n", (* (*ptr + i) )(a, b));
	}

	return 0;	
}
