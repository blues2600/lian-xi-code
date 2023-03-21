/*
  任务说明：实现一个功能与 getcwd()相当的函数。
  提示：要获得当前工作目录的名称，可调用
  opendir()和 readdir()来遍历其父目录（..）中的各个条目，查找其中与当前工作目录
  具有相同 i-node 编号及设备编号（即，分别为 stat()和 lstat()调用所返回 stat 结构中
  的 st_ino 和 st_dev 属性）的一项。如此这般，沿着目录树层层拾级而上（chdir("..")）
  并进行扫描，就能构建出完整的目录路径。当父目录与当前工作目录相同时（回忆
  /..与/相同的情况），就结束遍历。无论调用该函数成功与否，都应将调用者遣回其
  起始目录（使用 open()和 fchdir()能很方便地实现这一功能）。
*/

/*
    char *getcwd(char *buf, size_t size);
    getcwd()获取当前工作目录。
    getcwd()函数将内含当前工作目录绝对路径的字符串（包括结尾空字符）置于 cwdbuf 指
    向的已分配缓冲区中。调用者必须为 cwdbuf 缓冲区分配至少 size 个字节的空间。（通常，
    cwdbuf 的大小与 PATH_MAX 常量相当。）
*/

/*
    任务分解：
    0 打开.当前目录，使用stat查看当前目录的ino和dev信息
    1 opendir打开../目录
    2 readdir遍历该目录下除.和..的所有目录
    3 目录也是文件，使用stat和lstat查看文件的ino和dev信息，与之前的信息进行匹配
*/

#define _XOPEN_SOURCE 700
#define _POSIX_C_SOURCE 1

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/sysmacros.h>
#include <limits.h>
#include <stdbool.h>

typedef struct file_info {
    dev_t st_dev; /* 包含文件的设备ID */
    ino_t st_ino; /* 索引节点号 */
} FileInfo;

// 打开一个目录，并返回指向该目录的句柄
// 对opendir的封装
// 成功返回句柄，失败退出程序并输出错误提示
DIR *MyOpendir(const char *dirname)
{
    DIR *proc = opendir(dirname);
    if (proc == NULL) {
        printf("opendir() failed, %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    return proc;
}

// 返回文件的相关信息(stat结构体)
// 对stat的封装
// 成功返回零，失败退出程序并输出错误提示
int MyStat(const char *path, struct stat *buf)
{
    int retval = stat(path, buf);

    if (retval == -1) {
        printf("stat() failed in MyStat(), %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    return retval;
}

// 获得文件的设备ID和i节点号
// 对stat的封装，导出dev和ino
void GetFileInfo(const char *path, FileInfo *fi)
{
    struct stat *buf = (struct stat *)malloc(sizeof(struct stat));
    int retval = stat(path, buf);

    if (retval == -1) {
        printf("stat() failed in GetFileInfo(), %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    fi->st_dev = buf->st_dev;
    fi->st_ino = buf->st_ino;
    free(buf);
}

// 读取下一个目录,目录信息被拷贝到next_dir指向的内存
// 成功返回0，到达目录结尾返回-1，出错输出错误信息并退出程序
int ReadDir(DIR *path, struct dirent *next_dir)
{
    struct dirent *temp = readdir(path);  // 获得下一个目录
    if (temp == NULL && errno == 0)       // 到达目录流结尾
        return -1;
    if (temp == NULL && errno != 0)  // 出现错误
    {
        printf("readdir() failed in ReadDir, %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    memcpy(next_dir, temp, sizeof(struct dirent));
    return 0;
}

// 改变进程的当前工作目录
// 对chdir的封装
// 成功返回0，失败输出错误提示并退出程序
int MyChdir(const char *path)
{
    int retval = chdir(path);

    if (retval == -1) {
        printf("stat() failed in GetFileInfo(), %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    return retval;
}

// 比较两个FileInfo变量是否相等
// 本质上是比较设备号和i节点是否相等
// 相等返回0，否则返回非0
int CmpFileInfo(const FileInfo *a, const FileInfo *b)
{
    if (a->st_dev == b->st_dev && a->st_ino == b->st_ino)
        return 0;
    else
        return -1;
}

// 查找目录
// 根据设备号和i节点查找匹配的目录，目录名称通过result_name返回
// 成功返回0，失败返回-1，并输出错误信息
int FindDir(FileInfo *target, char *result_name)
{
    DIR *current_dir = NULL;
    struct dirent next_dir;
    FileInfo fi;

    // 打开当前目录，获得目录流
    current_dir = MyOpendir(".");

    // 搜索目录
    while (ReadDir(current_dir, &next_dir) != -1) {
        // 跳过.和..目录
        if (strcmp(next_dir.d_name, ".") == 0)
            continue;
        if (strcmp(next_dir.d_name, "..") == 0)
            continue;

        // 获得当前目录的i节点和设备号
        GetFileInfo(next_dir.d_name, &fi);

        // 比较i节点和设备号
        if (!CmpFileInfo(&fi, target)) {
            strcpy(result_name, next_dir.d_name);
            closedir(current_dir);
            return 0;
        }
    }

    closedir(current_dir);
    printf("FindDir():can not find the dir.\n");
    return -1;
}

// 判断当前目录是否为根目录
// 判断依据是，根目录的当前目录和父目录相同，即.和..目录指向相同
bool IsRoot()
{
    FileInfo da, db;
    GetFileInfo(".", &da);
    GetFileInfo("..", &db);

    if (da.st_dev == db.st_dev)
        if (da.st_ino == db.st_ino)
            return true;

    return false;
}

int main(int argc, char *argv[])
{
    FileInfo workdir_fileinfo;
    memset(&workdir_fileinfo, 0, sizeof(FileInfo));
    char current[NAME_MAX + 1] = {'\0'};
    char temp[NAME_MAX + 1] = {'\0'};
    char complete_dir[NAME_MAX + 1] = {'\0'};

    if (argc > 1) {
        printf("useage: app\n");
        exit(EXIT_FAILURE);
    }

    // 获取进程工作目录的i节点和设备号
    GetFileInfo(".", &workdir_fileinfo);
    // 测试文件信息是否正确
    //printf("当前工作目录的设备ID=%ld,i节点=%ld\n", (long)major(workdir_fileinfo.st_dev), (long)workdir_fileinfo.st_ino);

    while (1) {
        // 修改进程当前目录，跳到上级目录去
        MyChdir("../");

        // 查找匹配的目录
        if (!FindDir(&workdir_fileinfo, current)) {
            strcpy(temp, "/");
            strcat(temp, current);
            strcat(temp, complete_dir);
            strcpy(complete_dir, temp);
        }

        // 到了根目录
        if (IsRoot()) {
            printf("the current dir:%s\n", complete_dir);
            return 0;
        }

        GetFileInfo(".", &workdir_fileinfo);
    }

    printf("can not find pwd\n");
    return 0;
}
