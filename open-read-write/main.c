/*
 * tee 命令是从标准输入中读取数据，直至文件结尾，随后将数据写入
 * 标准输出和命令行参数所指定的文件。（44.7 节讨论 FIFO 时，会
 * 展示使用 tee 命令的一个例子。）请使用I/O 系统调用实现 tee 命
 * 令。默认情况下，若已存在与命令行参数指定文件同名的文件，tee 
 * 命令会将其覆盖。如文件已存在，请实现-a 命令行选项（tee-a file）
 * 在文件结尾处追加数据。（请参考附录 B 中对 getopt()函数的描述
 * 来解析命令行选项。）
 */

/*
 * 任务分解
 * 1.从标准输入读取数据，直到文件结尾
 * 2.将数据写入标准输出
 * 3.将数据写入命令行参数所指定的文件
 * 4.添加一个命令参数，在文件结尾处追加内容
 */

/*  经验总结
 *
 *  清空输入和输出缓冲区
    清空输出就是将缓冲区的内容现在全部输出
    fflush(stdout);
    int c;
    while((c = getchar()) != '\n' && c != EOF);

    open函数的权限控制有点复杂，而且需要进一步理解Linux用户、组、权限的概念

 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int write_file(char *newfile, char *buf, size_t n);
int get_data_from_stdin(char *buf);
int write_file_append(char *filename, char *buf, size_t n);

#define BUF_SIZE 1024

int main(int argc, char *argv[])
{
    if (argc < 2 || argc > 3) {
        printf("arg error\n");
        exit(EXIT_FAILURE);
    }

    char buf[BUF_SIZE];
    printf("enter your data:");
    fflush(stdout);
    get_data_from_stdin(buf);
    //printf("the data:%s", buf);

    if (argc == 3 && strcmp(argv[1], "-a") == 0)
        write_file_append(argv[2], buf, strlen(buf));
    else
        write_file(argv[1], buf, strlen(buf));
    return 0;
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

// 新建一个文件，将buf指向的n个字节内容写入文件newfile中，如果文件已经存在，则新文件覆盖旧文件
// 成功返回非0，失败返回0
int write_file(char *newfile, char *buf, size_t n)
{
    //新建文件
    int fd = open(newfile, O_WRONLY | O_CREAT | O_TRUNC,
                  S_IRWXU | S_IRUSR | S_IRGRP | S_IROTH);
    if (fd == -1) {
        printf("open() failed, %s\n", strerror(errno));
        return 0;
    }

    //向新文件写入数据
    ssize_t number = write(fd, buf, n);
    if (number == -1) {
        printf("write() failed, %s\n", strerror(errno));
        return 0;
    } else
        return 1;
}

// 从标准输入接收数据，保存到buf
// 成功返回非0，失败返回0
// 函数会在buf的有效字符末尾添加空结尾
int get_data_from_stdin(char *buf)
{
    ssize_t ReadNum = 0;

    // 经测试，回车不是EOF

    //从标准输入接收数据
    ReadNum = read(STDIN_FILENO, buf, BUF_SIZE - 1);

    if (ReadNum == -1) {
        printf("read() failed, %s\n", strerror(errno));
        return 0;
    }

    //到了EOF
    if (ReadNum == 0) {
        printf("it's EOF. \n");
        return 0;
    }

    //添加空结尾
    buf[ReadNum] = '\0';

    return 1;
}
