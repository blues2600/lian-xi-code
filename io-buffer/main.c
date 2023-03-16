

//1
//2

//

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

#define max(m, n) ((m) > (n) ? (m) : (n))
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

// 对lseek的封装，移动文件指针到开头
// 成功无返回，失败退出程序
void lseek_start(int fd)
{
    off_t retval = lseek(fd, 0, SEEK_SET);
    if (retval == -1) {
        printf("lseek() failed, %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

// 返回当前文件指针的位置
int cur_pos(int fd)
{
    if (fd <= 0) {
        printf("cur_pos(): error fd.\n");
        return 0;
    }

    off_t curpos = lseek(fd, 0, SEEK_CUR);
    if (curpos == -1) {
        printf("lseek() failed, %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    return curpos;
}

// 读取fd中len个字节的内容到buf
// 成功返回0，失败返回非0
int read_file(int fd, char *buf, int len)
{
    ssize_t num = 0;

    num = read(fd, buf, len);

    if (num == -1) {
        printf("read() failed, %s\n", strerror(errno));
        return 1;
    }
    if (num == 0) {
        printf("read_file() : it's EOF. \n");
        return 0;
    }

    return 0;
}

// buf是存储数据的缓冲器，limt为换行符集合
// 函数计算buf中总共有几行
// 成功返回行数，失败返回0，并打印错误信息
int how_many_line(char *buf, const char limt[])
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

//
int how_many_line2(char *buf,int len)
{
    int line = 0;
    for(int i=0;i<len;i++)
    {
        if(buf[i] == 0x0 || buf[i] == 0xA || buf[i] == 0xD)
            line++;
    }
    return line;
}

// 打印buf中的最后n行数据，换行符集合在limt中，lines是buf中现有行数
// 成功返回非0，失败返回0，并输出错误信息
int print_last_lines(char *buf, const char limt[], int n, int lines)
{
    int noprint = lines - n;
    int count = 0;
    char *token = strtok(buf, limt);

    if (token == NULL) {
        printf("strtok() failed in print_last_lines(). \n");
        return 0;
    }

    while (token != NULL) {
        count++;
        if (count > noprint)
            printf("%s\n", token);
        token = strtok(NULL, limt);
    }

    return 1;
}

// 获取文件(文件已经open)字节数
// 成功返回非0，失败返回0
int file_size(const int fd)
{
    if (fd <= 0) {
        printf("file_size(): error fd.\n");
        return 0;
    }

    int len = lseek(fd, 0L, SEEK_END);
    if (len == -1) {
        printf("seek() failed in file_size(), %s\n ", strerror(errno));
        return 0;
    }

    return len;
}

int main(int argc, char *argv[])
{
    int num_user;
    int num_cur = 0;
    int fd = -1;
    const char limt[] = "\xA\xD\x0";
    char *buf = (char *)malloc(BUFFER_SIZE);
    char *buf_cur = NULL;
    char *buf_new = NULL;

    if (argc > 3 || argc < 2) {
        printf("useage: app file [number of line]\n");
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
        num_user = atoi(argv[2]);

    fd = open_onlyread(argv[1]);  //打开文件

    // 将文件指针移动到适当的位置
    if (file_size(fd) > BUFFER_SIZE) {
        lseek_end(fd);
        lseek(fd, -BUFFER_SIZE, SEEK_CUR);  //向前移动
    } else
        lseek_start(fd);  //文件小于BUFFER_SIZE，移动到文件开头

    read_file(fd, buf, BUFFER_SIZE);     //从文件读取内容到buf
    num_cur = how_many_line(buf, limt);  //计算buf里有几行

    buf_cur = buf;
    //buf = NULL;
    int malloc_count = 1;          //堆内存分配次数
    while (num_cur <= num_user) {  //buf里的内容能否满足用户要求的行数？
        malloc_count++;
        buf_new = malloc(BUFFER_SIZE *
                         malloc_count);  //在原buf基础上扩充BUFFER_SIZE个字节
        if (buf_new == NULL) {
            printf("malloc() failed.\n");
            free(buf_cur);
            exit(EXIT_FAILURE);
        }
        //memcpy(buf_new + BUFFER_SIZE * (malloc_count - 1), buf_cur,BUFFER_SIZE * (malloc_count - 1));  //旧数据拷贝到新内存
        free(buf_cur);  //释放旧数据内存
        buf_cur = buf_new;

        // 将文件指针移动到适当的位置
        if (file_size(fd) > BUFFER_SIZE * malloc_count) {
            lseek_end(fd);
            lseek(fd, -(BUFFER_SIZE * malloc_count), SEEK_CUR);  //向前移动
        } else {
            lseek_start(fd);  //文件小于BUFFER_SIZE，移动到文件开头
            read_file(fd, buf_cur,
                      BUFFER_SIZE * malloc_count);  //从文件读取内容到buf
            num_cur = how_many_line2(buf_cur, file_size(fd));  //计算buf里有几行
            break;
        }

        read_file(fd, buf_cur, BUFFER_SIZE*malloc_count);     //从文件读取内容到buf
        num_cur = how_many_line2(buf_cur, BUFFER_SIZE*malloc_count);  //计算buf里有几行
    }

    // 现在buf_cur里面的数据行数已经满足了用户要求，拥有至少不少于num_user行数据
    // 输出用户请求的N行
    for (int i = 0; i < BUFFER_SIZE; i++) {
        if (buf_cur[i] == 0x0)
            buf_cur[i] = 0xA;
    }
    // 在堆里存储的数据，如果里面包含了换行字符中的任何一个
    // 在调用strtok之前调用了stdio函数，那么strtok就会不起作用
    // 表现为，它会将第一个换行字符前的数据认为是整个字符串
    print_last_lines(buf_cur, limt, num_user, num_cur);
    free(buf_cur);
    return 0;
}
