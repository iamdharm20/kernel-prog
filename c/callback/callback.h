/* function prototype */
struct math_operation {
	/* declaring a pointer to a function which takes two int arguments and returns an integer as result */
	int (*add)(int , int);
	int (*sub)(int , int);
	int (*mul)(int , int);
};

void register_callback(struct math_operation *);
