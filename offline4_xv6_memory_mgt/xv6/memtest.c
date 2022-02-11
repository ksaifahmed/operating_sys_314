#include "types.h"
#include "stat.h"
#include "user.h"


int main(int argc, char *argv[])
{
    printf(1, "user code, memtest init\n");

    int size = 4096 * 5, i;
    int *arr = (int *) malloc(sizeof(int) * size);
    printf(1, "user code, mem allocated\n");


    for(i=0; i<size; i++)
    {
        arr[i] = i * 2;
        if(i%100 == 0)
            printf(1, "user code, index: %d, val: %d\n", i, arr[i]);
    }

    printf(1, "hoise kaaj, sheeiiiiiiiiii\n");
	exit();
}