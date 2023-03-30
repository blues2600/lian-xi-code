/*
 * 任务
 * 命令 chmod a+rX file 的作用是对所有各类用户授予读权限，并且，当 file 是目录，
 * 或者 file 的任一用户类型具有可执行权限时，将向所有各类用户授予可执行权限。
 * 使用 stat()和 chmod()编写一程序，令其等效于执行 chmod a+rX 命令。
 */

/*
 * 思考
 * 让文件对所有用户授予读权限
 * 一个文件的权限信息，登记在文件i节点的mod字段低12位
 * 文件向所有用户授予读权限，即修改文件i节点的mod字段低9位其中的3位
 * 将这三个位置位
 *
 * 如果该文件是目录，则wrx权限的含义有所不同
 * 目录的x权限是访问目录文件或搜索权限
 *
 * 在修改文件的权限之前，首先判断文件是目录还是文件
 *
 */

/*
 * 思路
 * 1.从用户接收目标文件绝对路径
 * 2.判断文件是目录还是普通文件（IS_DIR）
 * 3.如果是文件直接改 chmod
 * 4.
 */

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    struct stat file;
    mode_t old_permissions = 0;
    mode_t new_permissions = 0;

    if (argc != 2) {
        printf("useage: ./app filepath\n");
        exit(EXIT_FAILURE);
    }

    // 获得文件信息
    if (stat(argv[1], &file)) {
        printf("stat() failed, %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    old_permissions = file.st_mode;

    if (S_ISDIR(file.st_mode)) {
        // 任意用户对目录具备执行权限，则所有用户对目录都赋予执行权限
        if (old_permissions & S_IXUSR || old_permissions & S_IXGRP ||
            old_permissions & S_IXOTH)
            old_permissions = old_permissions | S_IXUSR | S_IXGRP | S_IXOTH;

    new_permissions = old_permissions | S_IRUSR | S_IRGRP | S_IROTH;

    if (chmod(argv[1], new_permissions)) {
        printf("chmod() failed, %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    return 0;
}

