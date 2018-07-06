/* Array parameters treated as pointers because of efficiency. It is inefficient to copy the array data in terms of both memory and time; 
and most of the times, when we pass an array our intention is to just tell the array we interested in, not to create a copy of the array.
*/
#include <stdio.h>
void func(int arr[], int (*arr1)[], int *ptr, int num)
{
	/* valid why? Note: array parameters are treated as simple pointers */
	arr = NULL;
	arr1 = NULL;

	printf("arr:%p :arr1 %p : ptr:%p : num:%d : num:%p\n", arr, arr1, ptr, num, &num); 
}

int main()
{
	int arr[] = {1,2,3,4};
	//arr = NULL; /* error: assignment to expression with array type */

	func(arr, &arr, &arr[0], arr[0]);
	printf("arr:%p :arr[0]:%d\n", arr, arr[0]); 
}
