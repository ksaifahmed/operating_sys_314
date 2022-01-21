#include "types.h"
#include "stat.h"
#include "user.h"
//#include "defs.h"


int random_(int seed, int a, int c, int m){
  seed = (a * seed + c) % m ;
  return seed % 77;
}


int main(int argc, char *argv[])
{
	if(argc != 1){
		printf(1, "Invalid params!\n");
		exit();
	}

    int id = 0, i, j;
    double x = 1.01012;
    struct pstat *proc_info = (struct pstat*) malloc(sizeof(struct pstat));
    int alpha = 0;

    //parent creates first child
    id = fork();
    if(id < 0) printf(1, "Failed to fork first child!\n");
    if(!id) alpha = 1; //first child indicator!

    //parent creates second child
    if(id > 0) {
        id = fork();
        if(id < 0) printf(1, "Failed to fork second child!\n");
    }

    //parent creates third child
    if(id > 0) {
        id = fork();
        if(id < 0) printf(1, "Failed to fork second child!\n");
    }

    //parent inf loop
    if(id > 0){
        while(1)
        {
            if(getpinfo(proc_info) == -1){
                printf(1, "Something went wrong when retrieving process info!");
                wait();
            }
            printf(1, "PID\tTICKS\tTICKETS\n");
            for(i = 0; i < NPROC; i++){
                if(proc_info->pid[i] == 1 || proc_info->pid[i] == 2)
                 continue; //parent and shell process
                
                if(proc_info->inuse[i]){
                    printf(1, "%d\t%d\t%d\n", proc_info->pid[i], proc_info->ticks[i], proc_info->tickets[i]);
                }
            }            
            sleep(100);         
        }
    }

    //child does a heavy calculation
    if(!id){
        if(alpha) settickets(30);
        while(1)

        {
            for(j = 0; j < 214748000; j++)
                x += 0.0131*45.445;
        }
    }

	exit();
}
