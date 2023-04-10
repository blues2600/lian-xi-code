/*
 *
 *
 * 1.父进程变为僵尸
 * 2.观察子进程何时被收养，是在父进程终止后，还是祖父进程调用wait()后？
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


/*
 *
 * 实际测试结果，当父进程结束以后，子进程会立即收养（此时祖父进程还存活）
 *
 */

int main(int argc, char* argv[]) {
    switch (fork()) {
        case -1:
            errExit("fork");
        case 0:
            switch (fork()) {
                case -1:
                    errExit("fork");
                case 0:
                    /*
                     * 这里是子进程，这里需要先睡眠一会儿，完成等待操作系统调度
                     * 父进程退出，然后考察子进程被收养的状况
                     */
                    sleep(5);
                    printf("i am child , my ppid = %lld\n",(long long)getppid());
                default:
                    _exit(EXIT_SUCCESS);
            }
        default:
            /* 
             * 这里是祖父进程，当父进程调用_exit（），这里先测试不wait（）它
             * 所以它会称为僵尸
             */
            getchar();
            exit(EXIT_SUCCESS);
    }
    return 0;
}
