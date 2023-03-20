/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* t_unlink.c

    Demonstrate that, when a file is unlinked, it is not actually removed from
    the file system until after any open descriptors referring to it are closed.

    Usage: t_unlink file
*/
#include <sys/stat.h>
#include <fcntl.h>
#include "tlpi_hdr.h"

#define CMD_SIZE 200
#define BUF_SIZE 1024

int
main(int argc, char *argv[])
{
    int fd, j, numBlocks;
    char shellCmd[CMD_SIZE];            /* Command to be passed to system() */
    char buf[BUF_SIZE];                 /* Random bytes to write to file */

    if (argc < 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s temp-file [num-1kB-blocks] \n", argv[0]);

    // 将数字命令行参数字符串转换为整数
    numBlocks = (argc > 2) ? getInt(argv[2], GN_GT_0, "num-1kB-blocks")
                           : 100000;

    // 仅创建文件，创建者拥有可读写权限
    fd = open(argv[1], O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    if (fd == -1)
        errExit("open");

    // 删除硬链接，如果是最后一个硬链接，同时会删除文件
    if (unlink(argv[1]) == -1)          /* Remove filename */
        errExit("unlink");

    // 继续向刚才删除链接的文件写入数据
    // 在关闭所有文件描述符之前，系统实际上将不会删除该文件
    // 但是进程却可以继续使用这个文件
    for (j = 0; j < numBlocks; j++)     /* Write lots of junk to file */
        if (write(fd, buf, BUF_SIZE) != BUF_SIZE)
            fatal("partial/failed write");

    // 关闭fd之前查看文件信息
    snprintf(shellCmd, CMD_SIZE, "df -k `dirname %s`", argv[1]);
    system(shellCmd);                   /* View space used in file system */

    if (close(fd) == -1)                /* File is now destroyed */
        errExit("close");
    printf("********** Closed file descriptor\n");

    /* See the erratum for page 348 at http://man7.org/tlpi/errata/.
       Depending on factors such as random scheduler decisions and the
       size of the file created, the 'df' command executed by the second
       system() call below does may not show a change in the amount
       of disk space consumed, because the blocks of the closed file
       have not yet been freed by the kernel. If this is the case,
       then inserting a sleep(1) call here should be sufficient to
       ensure that the the file blocks have been freed by the time
       of the second 'df' command.
    */
    
    // 关闭fd之后查看文件信息
    system(shellCmd);                   /* Review space used in file system */
    exit(EXIT_SUCCESS);
}
