#include <stdio.h>
#include <string.h>
 
int main()
{
    int a[10];
    int *p;
    int i;
 
    for (i = 0; i < 10; i++)
        a[i] = 0x12345678;
    p = &a[9];
 
    for (i = 0; i < 10; i++) {
        printf("a[%d]=0x%x, %p\n", i, a[i], &a[i]);
        printf("p[-%d]=0x%x, %p\n", i, p[-i], &p[-i]);
    }
 
    return 0;
}
