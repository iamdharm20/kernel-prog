#include "little_big_indian.h"

void little_to_big_indian(int *nptr)
{
	int x = *nptr;

        printf("littleindian = 0x%x\n", *nptr);
        /*
        move byte 3 to byte 0
        move byte 1 to byte 2
        move byte 2 to byte 1
        byte 0 to byte 3
        */
        x = ( x >> 24 ) | ( ( x << 8) & 0x00ff0000 ) | ( (x >> 8) & 0x0000ff00 ) | ( x << 24)  ;
	
	*nptr = x;

        printf("bigindian = 0x%x\n", *nptr);  // x will be printed as 0x78563412
	
}


