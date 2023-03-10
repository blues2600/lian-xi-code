/*
 *
 * 编写一个类似于 cp 命令的程序，当使用该程序复制一个包含空洞（连续的空字节）
 * 的普通文件时，要求目标文件的空洞与源文件保持一致。
 *
 *
 */

/*
 * 大问题分解
 * 1.实现文件复制
 * 2.制造一个存在空洞的文件
 * 3.测试空洞是否被复制
 */

/*
 * 分解问题1
 * 1.打开源文件
 * 2.读取源文件数据
 * 3.创建新文件
 * 4.将数据写入新文件
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUF_SIZE 1024

int main(int argc, char *argv[])
{
    int source_fd = -1;
    char buf[BUF_SIZE];

    if (argc != 3) {
        printf("arg error\n");
        exit(EXIT_FAILURE);
    }

    // 打开被复制的文件
    source_fd = open_onlyread(argv[1]);

    while(read(source_fd,buf,BUF_SIZE)>0){  // 从文件中读取数据
        
    return 0;
}

// 以只读的方式打开一个文件
// 成功返回fd，失败退出程序
int open_onlyread(char *filename)
{
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        printf("open() failed, %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    return fd;
}

// 新建一个文件，如果文件存在，则覆盖它
int open_










