#include <stdio.h>

/* arr vs &arr and pointer : arr has the type int *, where as &arr has the type int (*)[size].
arr and &arr has the same address but (arr + 1) points to the second element of the array and &arr + 1) points to the memory address after the end of the array.
arr[1] = *(arr + 1);
*/

int main()
{
	int arr[] = {1,2,3,4,5};

	int arr1[5] = {1,2,3,4,5};

	int *ptr = arr;

	int *ptr1 = &arr;

	int (*ptr2) [5]= &arr1;
	
	int (*ptr3) [5]= arr1;


	printf("arr: %u &arr: %u arr+1:%u &arr + 1:%u\n", arr, &arr, arr + 1, &arr + 1);
	printf("arr: %d arr[0]:%d &arr[0]:%d\n", arr, arr[0], &arr[0]);
	
	printf("*(ptr + 1): %d *(ptr1 + 1): %d\n", *(ptr + 1), *(ptr1 + 1));

	printf("sizeofarr:%ld arr1:%ld &arr:%ld ptr:%ld *ptr1:%ld *ptr2 :%ld\n", sizeof(arr), sizeof(arr1), sizeof(&arr), sizeof(ptr), sizeof(*ptr1), sizeof(*ptr2));
	
	printf ("ptr:%d : ptr++:%d : ptr1:%d : ptr1++:%d : ptr2:%d: ptr2++:%d\n", ptr, ptr++, ptr1, ptr1++, ptr2, ptr2++);
//	printf ("*ptr++:%d : *ptr1++:%p : *ptr2++:%p\n", *ptr++, *ptr1++, *ptr2++);
	
	#if 0
		arr++;
		arr = arr + 1;
		arr = *ptr1;
	#endif

	return 0;
}
