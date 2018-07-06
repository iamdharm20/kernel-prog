#include "little_big_indian.h"

void check_indianness(char *ch)
{
        if (*ch) {
                printf("Little Endian\n");
        } else {
                printf("Big Endian\n");
        }
}

