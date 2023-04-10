/*
 *
 * 编写一程序以验证当一子进程的父进程终止时，调用 getppid()将返回 1（进程 init 的进程 ID）
 *
 *
 */

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "error_functions.h"

int main(int argc, char* argv[]) {

    switch (fork()) {
        case -1:
            errExit("fork");
        case 0:
            sleep(5);

            // 父进程退出以后，子进程会被init进程收养，于是getppid返回1

            /*
             *  
             *   实际测试，在Ubuntu 20.04.6 LTS中，子程序会被系统和会话管理器
             *   systemd收养，pid为2243，而非书上说的init
             */
            printf("child get ppid = %lld\n",(long long)getppid());
            _exit(EXIT_SUCCESS);
        default:
            exit(EXIT_SUCCESS);
    }
    return 0;
}
