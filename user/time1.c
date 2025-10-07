#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]){

    if(argc < 2){
        fprintf(2,"the usage time[args..]\n");
        exit(1);
    }

    int s = uptime(); 
    int pid = fork();

    if(pid<0){
        fprintf(2,"the error in fork");
        exit(1);
    }

    if(pid == 0){
        exec(argv[1], &argv[1]);
        fprintf(2,"exec %s error ",argv[1]);
        exit(1);
    }

    wait(0);
    int f = uptime();//finish time
    printf("elapsed time: %d ticks\n",f - s);

    exit(0);

}