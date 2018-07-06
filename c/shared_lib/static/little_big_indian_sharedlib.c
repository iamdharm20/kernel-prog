#include <stdio.h>
#include <dlfcn.h>

void check_indianness(char *);
void little_to_big_indian(int *nptr);

/*Main function to call above function for 0x01234567*/
int main()
{
        int var = 0x12345678;
	char *ch = (char *)&var;
        int *nptr = &var;
	
	void (*fn1)(char *);
	void (*fn2)(int *);

	char *error;
	
	void *lib_handle = dlopen("/home/dus5cob/gitweb/LKD/c/shared_lib/static/libindian.so.1.0", RTLD_LAZY);
	if (!lib_handle){
		fprintf(stderr, "%s\n", dlerror());
		return -1;
	}
	
	fn1 = dlsym(lib_handle, "check_indianness");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "%s\n", error);
		return -1;
	}

        /* check_indianness(ch);*/
	(*fn1)(ch);
	
        /*little_to_big_indian(nptr);*/
	fn2 = dlsym(lib_handle, "little_to_big_indian");
	if ((error = dlerror()) != NULL) {
		fprintf(stderr, "%s\n", error);
		return -1;
	}
	(*fn2)(nptr);
	
	printf("finally you learned the static and dynamic library creation\n");
        return 0;
}

