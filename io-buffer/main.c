/*
 * tail [ –n num ] file 命令打印名为 file 文件的最后 num 行（默认为 10 行）。
 * 使用 I/O系统调用（lseek()、read()、write()等）来实现该命令。牢记本章所描
 * 述的缓冲问题，力求实现的高效性。
 */

/*
 * 关于行的概念
 * 0x0  C语言空结尾
 * 0xA  ASCCI换行键
 * 0xD  ASCCI回车键
 *
 * 换行的判定
 * 1. 文本中出现以上任何一个字符，都视为换行
 * 2. 文本中出现以上任何一对组合，所谓组合，是以上任何两个或以上的字符连续的排列在一起，都视为换行（9种可能）
 */

/*
 * 程序思路
 * 1. 打开一个文件
 * 2. 将文件指针移动到文件末尾
 * 3. 从末尾开始read数据，在这里要考虑到IO缓冲的性能问题，如果一次从文件读取一个字节，那么1000个字节需要read（）1000次。如果一次从文件读取4096个字节，只需要read()一次。而对这些数据的操作，可以在缓冲区进行，这样就避免了系统调用和磁盘读写？？（略有迷糊）
 * 4. 在读取到的数据中，不断的寻找换行字符，并将所需要的内容打印
 */

/*
 * 程序设计
 *    保存用户要求的行数number
 * 1. 只读方式open一个文件
 * 2. lseek移动文件指针到末尾
 * 3. lseek向前移动文件指针4096个字节
 * 4. 使用read读取4096个字节数据
    5. 使用strtok查找一遍，确定有几行
			- 如果不够，再次读入4096个字节，并且将字符串拼接
			- 直到buf里面有超过10行的数据

 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFFER_SIZE 4096

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

// 对lseek的封装，移动文件指针到末尾
// 成功无返回，失败退出程序
void lseek_end(int fd)
{
    off_t retval = lseek(fd, 0, SEEK_END);
    if (retval == -1) {
        printf("lseek() failed, %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

// 读取fd中len - 1个字节的内容到buf
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

    return 1;
}

// buf是存储数据的缓冲器，limt为换行符集合
// 函数计算buf中总共有几行
// 成功返回行数，失败返回0，并打印错误信息
int how_many_line(char *buf, char limt[])
{
    int retval = 0;
    char *token = strtok(buf, limt);

    if (token == NULL) {
        printf("how_many_line() failed.\n");
        return 0;
    }

    while (token != NULL) {
        retval++;
        token = strtok(NULL, limt);
    }

    return retval;
}

int main(int argc, char *argv[])
{
    int num_user;
    int num_cur = 0;
    int fd = -1;
    const char limt[] = "\xA\xD\x0";
    char *token;
    char *buf = (char *)malloc(BUFFER_SIZE);
    char *buf_cur = NULL;
    char *buf_new = NULL;

    if (argc > 3 || argc < 2) {
        printf("user age: app file [number of line]\n");
        exit(EXIT_FAILURE);
    }

    if (buf == NULL) {
        printf("malloc() failed.\n");
        exit(EXIT_FAILURE);
    }

    // 确定要输出的行数
    if (argc == 2)
        num_user = 10;
    else
        num_user = atoi(rgv[2]);

    fd = open_onlyread(argv[1]);       //打开文件
    lseek_end(fd);                     //文件指针移动到末尾
    lseek(fd, BUFFER_SIZE, SEEK_CUR);  //向前移动4095字节

    read_file(fd, buf, BUFFER_SIZE);     //从文件读取内容到buf
    num_cur = how_many_line(buf, limt);  //计算buf里有几行

    buf_cur = buf;
    int malloc_count = 1;         //堆内存分配次数
    while (num_cur < num_user) {  //buf里的内容能否满足用户要求的行数？
        malloc_count++;
        buf_new = calloc(BUFFER_SIZE * malloc_count,
                         1);  //在原buf基础上扩充BUFFER_SIZE字节
        if (buf == NULL) {
            printf("malloc() failed.\n");
            exit(EXIT_FAILURE);
        }
        memcpy(buf_new - (BUFFER_SIZE - 1), buf_cur);  //旧数据拷贝
        free(buf_cur);                                 //释放旧数据内存
        buf_cur = buf_new;
    }

    return 0;
}
