#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

void print_usage() {

	printf("Usage: -i ioctl -r read -w write -t create thread -s stop thread\n");
}

int main(int argc, char *argv[]) 
{
	int option = 0;

	//Specifying the expected options
	//The two options l and b expect numbers as argument
	while ((option = getopt(argc, argv,"irwts:")) != -1) {
		switch (option) {
		case 'i' : 
			printf("%d\n", option);
			break;
		case 'r' :
			printf("%d\n", option);
			break;
		case 'w' :  
			printf("%d\n", option);
			break;
		case 't' :
			printf("%d\n", option);
			break;
		case 's' :
			printf("%d\n", option);
			break;
		default: 
			print_usage(); 
			exit(EXIT_FAILURE);
		}
	}
	return 0;
}

