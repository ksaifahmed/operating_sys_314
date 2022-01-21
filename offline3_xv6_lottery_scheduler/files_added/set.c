#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[])
{
	if(argc <= 1){
		printf(1, "Too few params!\n");
		exit();
	}
	if(argc == 2){
		settickets(atoi(argv[1]));
		exit();
	}
	printf(1, "Too many arguments!\n");
	exit();
}
