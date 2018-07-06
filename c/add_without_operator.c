#include<stdio.h>

/*Sum of two bits can be obtained by performing XOR (^) of the two bits. Carry bit can be obtained by performing AND (&) of two bits.
calculate (x & y) << 1 and add it to x ^ y to get the required result.
*/

int add4(int x, int y)
{
	x = x - (- y);

	return x;
}
int add3(int x, int y)
{
	while (y > 0) {
		x++;
		y--;
	}
	return x;
}
int add2(int x, int y)
{
    if (y == 0)
        return x;
    else
        return Add( x ^ y, (x & y) << 1);
}

int add(int x, int y)
{
	// Iterate till there is no carry  
	while (y != 0) {
		// carry now contains common set bits of x and y
		int carry = x & y;  
 
		// Sum of bits of x and y where at least one of the bits is not set
		x = x ^ y; 

		// Carry is shifted by one so that adding it to x gives the required sum
		y = carry << 1;
	}
	return x;
}
 
int main()
{
	int num1, num2;

	printf("enter two number\n");

	scanf("%d%d", &num1, &num2);
	printf("Add: %d\n", add(num1, num2));
	return 0;
}
