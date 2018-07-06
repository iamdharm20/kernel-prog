#include <stdio.h>

/*A special pointer type called the “void pointer” allows pointing to any variable type,
 but is limited by the fact that it cannot be dereferenced directly. we must to typcast into data type*/
int main()
{
	void *ptr = NULL;

	/*size of pointer is 8 byte. pointer size is dependent on archtecture*/	
	printf("%ld\n", sizeof(ptr));
	return 0;
}
