//
// Created by 15461 on 2023/1/22.
//
/* our main.c */
#include "ramfs.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#define LEN 4096
int fd[LEN];
int main() {
    init_ramfs();
    for (int i = 0; i < LEN; ++i) {
        assert((fd[i]= ropen("/////0/",O_CREAT|O_RDWR|O_TRUNC|O_WRONLY|O_RDONLY))==-1);
        assert((fd[i]= ropen("/////0",O_CREAT|O_RDWR))>=0);
    }
    for (int i = 0; i < LEN*LEN; ++i) {
        rwrite(fd[0],"/0",1);
    }
    for (int i = 0; i < LEN*LEN; ++i) {
        rclose(fd[0]);
        assert((fd[0]=ropen("/0",O_TRUNC|O_RDWR))>=0);
    }
    return 0;
}
