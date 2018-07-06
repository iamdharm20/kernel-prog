#include <stdio.h>

enum week {
	sun,
	mon,
	tus,
	wed,
	thr,
	fri,
	sat,
};

int main ()
{

	printf("%d\n",sun);
	printf("%ld\n", sizeof(enum week));

	//var.sun = 4; /* error: request for member ‘sun’ in something not a structure or union */
	return 0;
}
