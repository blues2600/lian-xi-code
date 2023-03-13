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

 /*
    总结：整个程序的功能已经完成了，或者说对于基本IO函数的训练目的达到了
    在kali下复制文本文件的时候还存在一些小缺陷，但现在暂且不管它了
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
// 成功返回fd，失败退出程序
// 如果新的文件名是一个已经存在的文件，并且它是符号链接，则函数调用失败，程序终止
int create_file(char *filename)
{
    int fd = open(filename,O_WRONLY | O_CREAT | O_TRUNC,
                  S_IRWXU | S_IRUSR | S_IRGRP | S_IROTH);
    if (fd == -1) {
        printf("open() failed, %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    return fd;
}

// 在一个已存在的文件（filename）中追加(buf)内容（n）个字节
// 成功返回非0，失败返回0
int write_file_append(char *filename, char *buf, size_t n)
{
    int fd = open(filename, O_WRONLY | O_NOFOLLOW | O_APPEND);
    if (fd == -1) {
        printf("open() failed, %s\n", strerror(errno));
        return 0;
    }

    //向已经存在的文件写入数据
    ssize_t number = write(fd, buf, n);
    if (number == -1) {
        printf("write() failed, %s\n", strerror(errno));
        return 0;
    } else
        return 1;
}

int main(int argc, char *argv[])
{
    int source_fd = -1;
    int target_fd = -1;
    char buf[BUF_SIZE];

    if (argc != 3) {
        printf("arg error\n");
        exit(EXIT_FAILURE);
    }

    // 打开被复制的文件
    source_fd = open_onlyread(argv[1]);
    // 新建一个文件，存放被复制的数据
    target_fd = create_file(argv[2]);
    close(target_fd);

    while(read(source_fd,buf,BUF_SIZE)>0){  // 从文件中读取数据

        /*
                测试代码
        printf("buf=%s\n",buf);
        */

        // 以追加的方式不断往新文件内复制数据
        if(!write_file_append(argv[2], buf, BUF_SIZE))
            exit(EXIT_FAILURE);
    }
        
    return 0;
}

