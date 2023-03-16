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

// 获得当前文件指针的位置
// 成功返回pos位置，否则程序退出
int cur_pos(int fd)
{
    if (fd <= 0) {
        printf("cur_pos(): error fd.\n");
        exit(EXIT_FAILURE);
    }

    off_t curpos = lseek(fd, 0, SEEK_CUR);
    if (curpos == -1) {
        printf("lseek() failed, %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    return curpos;
}

// 读取文件内容到缓冲区
// 读取fd中len个字节的内容到buf
// 成功返回读取的字节数，失败返回非0
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

    return num;
}

// 作废，不好用
// buf是存储数据的缓冲器，limt为换行符集合
// 函数计算buf中总共有几行
// 成功返回行数，失败返回0，并打印错误信息
//int how_many_line(char *buf, const char limt[])
//{
//    int retval = 0;
//    char *token = strtok(buf, limt);
//
//    if (token == NULL) {
//        printf("how_many_line() failed.\n");
//        return 0;
//    }
//
//    while (token != NULL) {
//        retval++;
//        token = strtok(NULL, limt);
//    }
//
//    return retval;
//}

// 计算数据块中有几个换行符
// 逐个字节扫描0x0/0xA/0xD
// 返回换行符数量，失败返回-1
int how_many_line(char *buf, int len)
{
    if (buf == NULL || len <= 0) {
        printf("how_many_line() failed, buf is null or len <= 0\n");
        return -1;
    }

    int line = 0;
    for (int i = 0; i < len; i++) {
        if (buf[i] == 0x0 || buf[i] == 0xA || buf[i] == 0xD)
            line++;
    }
    return line;
}

// 作废，不好用
// 打印一个数据块中的最后N行
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

// 向stdout输出数据块
// 对write的封装
// 成功返回输出的字节数，失败返回-1
int write_data(int *buf, size_t len)
{
    ssize_t number = write(STDOUT_FILENO, buf, len);
    if (number == -1) {
        printf("write() failed in write_data(), %s\n", strerror(errno));
        return -1;
    } else
        return number;
}

// 输出一个数据块中的最后n行
// 每一个换行符(0x0/0xA/0xD)都会导致输出换行
// 如果buf为NULL或len<=0，函数打印错误消息
void print_block(char *buf, int len, int n)
{
    char *head = buf;
    char *end = NULL;

    //参数检查
    if (buf == NULL || len <= 0)
        printf("print_block() failed, buf is null or len <= 0.\n");

    //计算数据块有几行
    int lines = how_many_line(buf, len);

    for (int i = 0; i < len; i++) {
        if (buf[i] == 0x0 || buf[i] == 0xA || buf[i] == 0xD) {
            end = buf + i;
            lines--;
            if (lines <= n - 1) {
                if (*head == 0xa)
                    printf("\n");
                else {
                    printf("%s\n", head);
                    break;
                }
            }
            head = end + 1;
        }
    }
}

// 获取文件的字节数
// 文件必须已经打开
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
    char *buf_cur = NULL;
    char *buf_new = NULL;

    if (argc > 3 || argc < 2) {
        printf("useage: app file [number of line]\n");
        exit(EXIT_FAILURE);
    }

    char *buf = (char *)malloc(BUFFER_SIZE);
    if (buf == NULL) {
        printf("malloc() failed.\n");
        exit(EXIT_FAILURE);
    }

    // 确定要输出的行数
    if (argc == 2)
        num_user = 10;
    else
        num_user = atoi(argv[2]);

    // 打开文件
    fd = open_onlyread(argv[1]);

    // 将文件指针移动到适当的位置
    if (file_size(fd) > BUFFER_SIZE) {
        lseek_end(fd);
        lseek(fd, -BUFFER_SIZE, SEEK_CUR);  //向前移动
    } else
        lseek_start(fd);  //文件小于BUFFER_SIZE，移动到文件开头

    //从文件读取内容到buf
    int buf_size = read_file(fd, buf, BUFFER_SIZE);

    // 计算buf里有几行数据
    num_cur = how_many_line(buf, BUFFER_SIZE);

    //堆内存分配次数
    buf_cur = buf;
    int malloc_count = 1;

    // 检查buf里的内容能否满足用户要求的行数
    while (num_cur <= num_user) {
        // 将现有buf内存扩充BUFFER_SIZE大小
        malloc_count++;
        buf_new = malloc(BUFFER_SIZE * malloc_count);
        if (buf_new == NULL) {
            printf("malloc() failed.\n");
            free(buf_cur);
            exit(EXIT_FAILURE);
        }
        free(buf_cur);  //释放旧数据内存
        buf_cur = buf_new;

        // 将文件指针移动到适当的位置
        // 如果文件字节数小于buf，则从文件开头读取整个文件
        // 否则从文件末尾向前移动N个字节
        if (file_size(fd) > BUFFER_SIZE * malloc_count) {
            lseek_end(fd);
            lseek(fd, -(BUFFER_SIZE * malloc_count), SEEK_CUR);  //向前移动
        } else
            lseek_start(fd);

        // 从文件读取数据到buf
        buf_size = read_file(fd, buf_cur, BUFFER_SIZE * malloc_count);

        // 计算当前buf中有几行内容
        num_cur = how_many_line(buf, BUFFER_SIZE * malloc_count);
    }

    // 输出buf中的最后N行数据
    print_block(buf_cur, buf_size, num_user);

    free(buf_cur);
    close(fd);
    return 0;
}
