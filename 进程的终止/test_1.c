/*
 *
 * 错误处理：
 * 1.有errno，使用errMsg()，输出错误信息格式：errMsg("open ()");//ERROR [ENOENT No such file or directory] open ()
 * 2.有errno，使用errExit()，程序退出，错误信息格式同上 
 * 3.无errno，使用其他自定义错误输出
 *
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
#include <sys/wait.h>

#include "error_functions.h"

int main(int argc, char* argv[]) {

    pid_t child = 0;
    printf("新建一个子进程\n");

    switch (fork()) {
        case -1:
            errExit("fork");
        case 0: //子进程执行
            printf("i am child process pid: %lld\n",(long long)getpid());
            _exit(0); // 在创建子进程的应用中，典型情况下仅有一个进程（一般为父进程）
                     //应通过调用 exit()终止，而其他进程应调用_exit()终止
        default:
            sleep(3);
            child = wait(NULL);
            if (child == -1)  
                errExit("wait()");
            printf("现在父进程苏醒了,子进程pid: %lld\n",(long long)child);
    }
    return 0;
}
