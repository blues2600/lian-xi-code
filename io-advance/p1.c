/*
 *
 * 编写一个程序，使用 O_APPEND 标志并以写方式打开一个已存在的文件，且将
 * 文件偏移量置于文件起始处，再写入数据。数据会显示在文件中的哪个位置？为
 * 什么？
 *
 */

/*
 *
 * 打开文件
 * 设置文件指针
 * 向文件写入数据
 *
 */

/*
 *
 * 基于让代码更简单，让程序更简单的原则，取消了之前对于系统调用的封装
 * 之前将系统调用后添加错误检测并封装成自己的函数
 * 这个办法隐藏了系统调用接口，不仅增加了阅读成本，而且增加了程序的运行开销
 * 改成现在这样直接写的方式，虽然增加了代码行数，看起来更复杂，实际上是更为精简了
 *
 */

/*
 *
 * 总结：
 * 程序运行结果
 * 输入的数据被写到了文件的末尾
 * 即使将文件内容指针移动到了文件的开头
 * 但是实际上受到O_APPEND 标志的影响
 * 数据仍然被追加到文件的末尾
 *
 *
 * 另一点是，使用 O_APPEND 标志在一定程度上可以避免竞争条件的发生
 * 但是这种避免只是某种程度上的，在确保写入数据的原子性操作这一点上，需要特别注意
 */


#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
    int fd = -1;
    char buf[100] = {'\0'};

    // 打开文件
    fd = open("./test.txt", O_WRONLY | O_APPEND);
    if (fd == -1) {
        printf("open() failed, %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // 移动文件指针到文件的开头
    if (lseek(fd, 0, SEEK_SET) == -1) {
        printf("lseek() failed, %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    printf("enter your content:");
    scanf("%s", buf);

    if(write(fd, buf, strlen(buf)) == -1){
        printf("lseek() failed, %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    return 0;
}
