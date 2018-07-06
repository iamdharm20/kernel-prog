#include <stdio.h>
/* Note : first compiler check the maximum data type then it start giving the block of that maximum data type
here maximum data type is long double i.e 16B so total size is 64B => 16B[num + c + d] + 16B[ch + num1] + 16B[ld] + 16B[num3]
*/
struct data {
	int num;
	char c;
	double d;
	char ch;
	int num1;
	long double ld;
	int num3;
};


int main()
{
	printf("sizeof short int : %ld\n", sizeof(short int));
	printf("sizeof unsigend int : %ld\n", sizeof(unsigned int));
	printf("sizeof unsigned long: %ld\n", sizeof(unsigned long));
	printf("sizeof unsigned long long: %ld\n", sizeof(unsigned long long));
	printf("sizeof double: %ld\n", sizeof(double));
	printf("sizeof long double: %ld\n", sizeof(long double));
	
	printf("sizeof struct data: %ld\n", sizeof(struct data));	
	return 0;
}
