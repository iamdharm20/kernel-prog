#include <stdio.h>

struct test {
	int 		a;
	unsigned long 	b;
	char 		c;
};

struct test function(unsigned long val)
{
	return (struct test) { .a = -23492,
			     .b = val,
			     .c = 'z' };
}

int main()
{
	unsigned long tvz = 7;
	struct test stt = function(tvz);
	printf("%d, %lu, %c\n", stt.a, stt.b, stt.c);

	return 0;	
}
