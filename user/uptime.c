#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[]){

    int time = uptime();
    printf("up %d clock is ticking\n", time);
    exit(0);
}