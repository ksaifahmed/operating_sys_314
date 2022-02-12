#include "types.h"
#include "stat.h"
#include "user.h"


int main(int argc, char *argv[])
{
    printf(1, "parent user code, memtest init\n");

    int size = 4096 * 18, i;
    int *arr = (int *) malloc(size);
    printf(1, "parent code, mem allocated\n");

    int pid = fork();

    for(i=0; i<size/4; i++)
    {
        arr[i] = i * 2;
        if(i%100 == 0){
            if(pid == 0)
                printf(1, "parent code, index: %d, val: %d\n", i, arr[i]);
            else
                printf(1, "child code, index: %d, val: %d\n", i, arr[i]);        
        }
        sleep(1);
    }

    free((void *)arr);

    if(pid != 0){
        wait();
        printf(1, "hoise kaaj, parent code ending!\n");        
    }else{
        printf(1, "hoise kaaj, child code ending!\n");
    }


	exit();
}