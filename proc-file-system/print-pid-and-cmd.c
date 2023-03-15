/*
 * 编写一个程序，以用户名作为命令行参数，列表显示该用户下所有正在运行的进程
 * ID 和命令名。（程序清单 8-1 中的 userIdFromName()函数对本题程序的编写可
 * 能会有所帮助。）通过分析系统中/proc/PID/status 文件的 Name：和 Uid：各行
 * 信息，可以实现此功能。遍历系统的所有/proc/PID 目录需要使用 readdir(3)函数，
 * 18.8 节对其进行了描述。程序必须能够正确处理如下可能性：在确定目录存在与
 * 程序尝试打开相应/proc/PID/status 文件之间，/proc/PID 目录消失了。
 */

/*
 * 任务整理
 * 1.显示用户所属的进程ID和命令字符串
 * 2.userIdFromName()函数会有帮助
 * 3.实现：通过分析系统中/proc/PID/status 文件的 Name：和 Uid
 * 4.遍历系统的所有/proc/PID 目录需要使用 readdir(3)函数
 * 5.问题：确定目录存在，但程序尝试打开相应/proc/PID/status 文件之间，/proc/PID 目录消失了
 */

/*
 * 程序思路
 * 1.通过userIdFromName查询UID
 * 2.opendir打开目录，readdir()遍历目录（当readdir返回NULL且errno不变时，表示到达了目录尾，而非目录消失了。注意在调用之前需要设置errno。）
 * 3.根据遍历得到的目录名，open每个PID的status文件
 * 4.读取UID字段的值，若真实ID匹配则输出进程ID，并读取/pid/cmdline的内容后输出
 */

/*
 * 程序设计
 * 1.userIdFromName得到UID
 * 2.opendir得到目录流
 * 3.readdir遍历目录（当返回NULL且errno不变，结束循环），获得目录名
 * 4.根据目录名open对应的status文件
 * 5.读取文件中的uid，与UID进行对比
 * 6.uid不匹配，readdir遍历下一个目录；uid匹配，输出pid，open /proc/pid/cmdline，读取并打印
 */

#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <ctype.h>

#define CONTENT_MAX_LEN 1024

// 通过用户名返回用户ID
// 成功返回用户ID，失败退出程序
uid_t userIdFromName(const char *name)
{
    struct passwd *pwd;
    uid_t u;
    char *endptr;

    if (name == NULL || *name == '\0') {
        printf("name is empty, quit.\n");
        exit(EXIT_FAILURE);
    }

    u = strtol(name, &endptr, 10); /* As a convenience to caller */
    if (*endptr == '\0')           /* allow a numeric string */
        return u;

    errno = 0;
    pwd = getpwnam(name);
    if (pwd == NULL) {
        printf("getpwnam() failed, %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    return pwd->pw_uid;
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

// 读取fd中len - 1个字节的内容到buf
// 函数会在buf的有效字符末尾添加'\0'
// 成功返回0，失败返回非0
int read_file(int fd, char *buf, int len)
{
    ssize_t num = 0;

    num = read(fd, buf, len - 1);

    if (num == -1) {
        printf("read() failed, %s\n", strerror(errno));
        return 0;
    }
    if (num == 0) {
        printf("it's EOF. \n");
        return 0;
    }

    buf[num] = '\0';
    return 1;
}

// 检查一个字符串是否全是数字
// 如果字符串全由数字组成，则返回0，否则返回非0
int is_all_number(char *str)
{
    if (*str == '\0') {
        printf("the string is empty.\n");
        return 1;
    }
    while (*str != '\0') {
        if (!isdigit(*str))
            return 1;
        str++;
    }
    return 0;
}

// 读取并分析buf每一行（换行符为'\n'）
// 当某一行的最前面n个字符和name指向的字符串完全相同时
// 这一行的数据被拷贝到result中，函数返回0
// 其他情况下，函数返回非0，同时result置为NULL
int find_str(char *buf, int n, char *name, char *result, int resultLen)
{
    char *token = strtok(buf, "\n");  //返回第一个token
    while (token != NULL) {
        if (!strncmp(token, name, n)) {         //判断是否相同
            strncpy(result, token, resultLen);  //相同则拷贝这行数据
            return 0;
        }
        token = strtok(NULL, "\n");  // 寻找下一行
    }

    result = NULL;
    return 1;
}

// 从stutas文件的uid数据行中提取real uid
// 成功返回非0，失败返回0
int get_real_uid(char *buf, int *uid)
{
    char *token = strtok(buf, "\t");  //返回第一个token
    if (token != NULL) {
        token = strtok(NULL, "\t");  // 获得real uid
        *uid = atoi(token);          // real uid转换为int
        return 1;
    }

    printf("can not get real uid.\n");
    return 0;
}

// 为了解决读取到/proc/pid/cmdline文件中的命令行参数问题
// 参数的后面包含的0x0字符（它会导致printf停止输出后续的内容）
// 例如ls -l
// printf在输出read接收到的数据时，会直接输出ls，而抛弃-l
// 为了解决这个问题，函数设定，当它遇到一个单独的0x0字节，则将它换成空格（0x20）
// 当如果有两个或两个以上连续的0x0字节，则不改动
void clear_zero(char *buf,int len)
{
    for(int i=0;i<len;i++)
    {
        if(buf[i] == 0x0 && buf[i+1] != 0x0)
            buf[i] = 0x20;
    }
}

int main(int argc, char *argv[])
{
    uid_t userID;
    DIR *proc = NULL;
    struct dirent *nextDir = NULL;
    char status[] = "/status";
    char statusContent[CONTENT_MAX_LEN];
    char cmdlineContent[CONTENT_MAX_LEN];
    char result[CONTENT_MAX_LEN];

    if (argc < 2 || argc > 2) {
        printf("useage : app username\n");
        exit(EXIT_FAILURE);
    }

    // 通过用户名获得用户id
    userID = userIdFromName(argv[1]);
    printf("the user id=%d\n", userID);

    // 打开目录，获得目录流
    proc = opendir("/proc");
    if (proc == NULL) {
        printf("opendir() failed, %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // 遍历目录
    char cmdline_path[NAME_MAX];
    while (1) {
        int status_fd = -1;
        int cmdline_fd = -1;
        int uid = -1;
        errno = 0;
        char status_path[NAME_MAX] = "/proc/";

        nextDir = readdir(proc);            // 获得下一个目录
        if (nextDir == NULL && errno == 0)  // 到达目录流结尾
            break;
        if (nextDir == NULL && errno != 0)  // 出现错误
        {
            printf("readdir() failed, %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        // 跳过隐藏目录
        if (nextDir->d_name[0] == '.')
            continue;
        // 检查是否为pid目录
        if (is_all_number(nextDir->d_name))
            continue;

        // 生成目录名 /proc/[PID]
        strncat(status_path, nextDir->d_name, strlen(nextDir->d_name));

        // 生成文件名 /proc/[PID]/cmdline
        strcpy(cmdline_path,status_path);
        strcat(cmdline_path,"/cmdline");

        // 生成文件名 /proc/[PID]/status
        strncat(status_path, status, strlen(status));

        // 读取status文件内容
        status_fd = open_onlyread(status_path);
        read_file(status_fd, statusContent, CONTENT_MAX_LEN);

        // 在status文件中取出Uid这一行数据
        find_str(statusContent, 4, "Uid:", result, CONTENT_MAX_LEN);
        // 从result中取出uid，并将字符uid转换为int uid
        get_real_uid(result, &uid);

        // 当uid和用户输入的uid一致，输出pid和cmdline
        if (uid == (int)userID) {
            // 读取cmdline文件内容并输出
            cmdline_fd = open_onlyread(cmdline_path);
            read_file(cmdline_fd,cmdlineContent,CONTENT_MAX_LEN);
            printf("pid %s cmdline:\n", nextDir->d_name);
            clear_zero(cmdlineContent,CONTENT_MAX_LEN);
            printf("%s\n",cmdlineContent);
            printf("\n\n");
        }

        close(status_fd);
        close(cmdline_fd);
        result[0] = '\0';
        statusContent[0] = '\0';
        cmdlineContent[0] = '\0';
    }

    return 0;
}
