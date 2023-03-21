/*
 * 任务：遍历任意目录树，打印目录树所有文件的具体类型（目录、普通文件、符号连接）
 */

#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ftw.h>
#include <errno.h>

int ShowFileType(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf){
    printf("%s\t",(typeflag == FTW_F)?"normal":
                (typeflag == FTW_D)?"dir":
                (typeflag == FTW_SL)?"link":"???");

    printf("%d\t",ftwbuf->level);
    printf("%ld\t\t",sb->st_ino);
    printf("%-53s\n",fpath);

    return 0;
}

int main(int argc, char *argv[])
{
    int retval = -1;

    if (argc > 2) {
        printf("Usage: ./app path\n");
        exit(EXIT_FAILURE);
    }

    printf("Type\tDepth\tiNumber\t\tPath\n");
    printf("------------------------------------\n");

    if (argc == 1)
        retval = nftw(".", ShowFileType, 15, FTW_PHYS | FTW_MOUNT);
    else
        retval = nftw(argv[1], ShowFileType, 15, FTW_PHYS | FTW_MOUNT);

    if(retval != 0){
        printf("nftw() failed, %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    return 0;
}
