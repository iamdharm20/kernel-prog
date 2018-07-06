/* gcc –Wall -g –save-temps filename.c –o filename : pre-processing->compilation->Assembly->linkingi
gcc -E filename.c -o  filename.i 
gcc -S filename.c -o filename.s
gcc -c filename.c -o filename.o
gcc filname.c -o filname.out

debugging utility: gdb -tui filename, file, size, objdump, ldd,
*/

#include <stdio.h>

int main()
{
	printf("gcc compilation stages\n");
	return 0;
}
