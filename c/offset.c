#include <stdio.h>
#include <stddef.h>
 
/* gcc attribute ((packed)) is used to tell the compiler not to place any "memory holes" within a structure.*/
struct foo {
         char  a;
         short b;
         int   c;
}__attribute__((packed));

#define OFFSET_A  offsetof(struct foo, a)
#define OFFSET_B  offsetof(struct foo, b)
#define OFFSET_C  offsetof(struct foo, c)
 
int main ()
{
         printf ("offset A = %ld\n", OFFSET_A);
         printf ("offset B = %ld\n", OFFSET_B);
         printf ("offset C = %ld\n", OFFSET_C);
         return 0;
}
