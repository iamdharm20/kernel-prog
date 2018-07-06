/* https://davejingtian.org/2017/08/03/pitfalls-in-negative-indexing-in-c/ */
/* arr[x] = *(arr + x) and arr[-2] = *(arr - 2) 
If one is sure that the elements exist, it is also possible to index backwards in an array; p[-1], p[-2], 
and so on are syntactically legal, and refer to the elements that immediately precede p[0]. Of course, 
it is illegal to refer to objects that are not within the array bounds.

	int arr[10];
	int x = arr[-2]; // invalid; out of range

	But this would be okay:
	int arr[10];
	int* p = &arr[2];
	int x = p[-2]; // valid:  accesses arr[0]

Principles of Compiler design:
Base Address of your Array a + (index of array *size of(data type for array a))
Base Address of your Array a + (5*size of(data type for array a))
i.e. 1000 + (5*2) = 1010
This explanation is also the reason why negative indexes in arrays work in C.
i.e. if I access a[-5] it will give me
Base Address of your Array a + (index of array *size of(data type for array a))
Base Address of your Array a + (-5 * size of(data type for array a))
i.e. 1000 + (-5*2) = 990

*/

#include <stdio.h>
#include <string.h>
 
int main()
{
    char a[10];
    char b[10];
 
    memset(a, 0x0, 10);
    memset(b, 0x1, 10);
 
    int i;
    printf("a=%p, b=%p\n", a, b);
    for (i = 0; i < 10; i++) {
        printf("b[%d]=%d, %p\n", i, b[i], &b[i]);
        printf("b[-%d]=%d, %p\n", i, b[-i], &b[-i]);
    }
 
    return 0;
}
