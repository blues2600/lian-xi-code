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

#include "error_functions.h"

int main(int argc, char* argv[]) {
    return 0;
}
