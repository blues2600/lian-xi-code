/*
 * 试用 execve()实现 execlp()。
 * 需使用 stdarg(3) API 来处理 execlp()所提供的变长参数列表。
 * 还需要使用 malloc 函数库中函数为参数以及环境向量分配空间。最后，请注意，要检
 * 查特定目录下某个文件是否存在且可以执行，有一种简单方法：尝试执行该文件即可。
 *
 * int execve(const char *pathname, char *const argv[], char *const envp[]);
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
    return 0;
}
