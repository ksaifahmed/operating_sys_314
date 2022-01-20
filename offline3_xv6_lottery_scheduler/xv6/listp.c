#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"

int main(int argc, char *argv[])
{
	if(argc != 1){
		printf(1, "Invalid no of params!\n");
		exit();
	}

    struct pstat *proc_info = (struct pstat*) malloc(sizeof(struct pstat));
    if(getpinfo(proc_info) == -1){
        printf(1, "Something went wrong when retrieving process info!");
        exit();
    }

    int i;
    printf(1, "PID\tTICKS\tTICKETS\n");
    for(i = 0; i < NPROC; i++){
        if(proc_info->inuse[i]){
            printf(1, "%d\t%d\t%d\n", proc_info->pid[i], proc_info->ticks[i], proc_info->tickets[i]);
        }
    }

	exit();
}
