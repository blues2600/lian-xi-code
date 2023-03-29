/*
 *
 *
 * 编写一程序，验证文件描述符及其副本是否共享了文件偏移量和打开文件的状态标志。
 *
 *
 */

/*
 *
 *  获得文件描述符的副本
 *  获得文件描述符的状态标志
 *  获得当前文件指针的位置
 *
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// 获得当前文件指针的位置
// 成功返回pos位置，否则程序退出
off_t GetPosition(int fd) {
    if (fd < 0) {
        printf("GetPosition(): error fd.\n");
        exit(EXIT_FAILURE);
    }

    off_t curpos = lseek(fd, 0, SEEK_CUR);
    if (curpos == -1) {
        printf("lseek() failed, %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    return curpos;
}

int main() {
    int fd = -1;
    int fd2 = -1;
    int flags = 0;
    int flags2 = 0;
    off_t pos = -1;
    off_t pos2 = -1;

    // 可写和末尾追加模式打开文件
    fd = open("./test.txt", O_WRONLY | O_APPEND);
    if (fd == -1) {
        printf("open() failed, %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // 获得fd打开文件时使用的文件标志
    if ((flags = fcntl(fd, F_GETFL)) == -1) {
        printf("fcntl() failed, %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // 复制文件描述符，若成功则两个描述符指向同一个系统级打开文件列表的条目
    fd2 = dup(fd);
    if (fd2 == -1) {
        printf("dup() failed, %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // 获得fd2打开文件时使用的文件标志
    if ((flags2 = fcntl(fd2, F_GETFL)) == -1) {
        printf("fcntl() failed, %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // 对比被复制的文件描述符是否共享了打开文件标志
    if (flags == flags2)
        printf("两个文件描述符共享了文件打开标志\n");
    else
        printf("两个文件描述符没有共享文件打开标志\n");

    // 获得当前文件指针的值
    pos = GetPosition(fd);
    pos2 = GetPosition(fd2);
    if (pos != pos2)
        printf("两个文件描述符指向的文件的文件指针不一致\n");
    else
        printf("两个文件描述符指向的文件的文件指针是一致的\n");

    return 0;
}

