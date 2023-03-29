/*
 *
 * 使用 read()、write()以及 malloc 函数包（见 7.1.2 节）中的必要函数以实现
 * readv() 和 writev()功能。
 *
 */

/*
 *
 * 对readv行为的理解
 * ssize_t readv(int fd, const struct iovec *iov, int iovcnt);
 * readv从fd指向的文件中读取数据。数据保存到哪里？读取的数据量多大？
 * iov是一个结构体，它的一个成员指向buf起始地址，另一个成员说明buf的长度
 * 所以，readv的数据保存到iov.iov_base的指向的地址，数据量为buf长度
 * 此外，readv会从iov[0]开始写入数据，一直到写满所有缓冲区，或者文件到达EOF
 *
 */

/*
 * 程序设计
 * 1.接收命令，useage: ./app BufferCount BufferLength
 * 2.只读模式打开文件（现在没有能力实现原子操作避免竞争条件）
 * 3.依次把数据read到iov的缓冲区里面
 *
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

// 分配IOVector内存
// amount = iovec数组元素个数
// length = 每个iovec元素指向的缓冲区长度
// 成功返回iovec数组指针，否则返回NULL
// 注意：使用完本函数之后，需要调用一次free()释放堆内存
/*
struct iovec* IOVectorMalloc(const int amount, const int length) {
    // 为缓冲区本身分配内存，采用一次性分配的策略
    char* temp = (char*)malloc(amount * length);
    if (temp == NULL) {
        printf("malloc() failed.\n");
        exit(EXIT_FAILURE);
    }

    char* buf = temp;
    struct iovec* array;

    // 虽然array是一个指针，但是这里将它当成一个数组来对待
    for (i = 0; i < amount; i++, array++) {
        array.iov_base = buf;
        array.iov_len = length;
        buf = buf + length;  // buf指向下个iovec元素的buf的起始地址
    }

    return array;
}
*/
/*
ssize_t MyReadv(int fd, const struct iovec *iov, int iovcnt)
{
}
*/

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("useage: ./app BufferCount BufferLength\n");
        exit(EXIT_FAILURE);
    }

    int amount = atoi(argv[1]);
    int length = atoi(argv[2]);

    // 为缓冲区本身分配内存，采用一次性分配的策略
    char* buf = (char*)malloc(amount * length);
    if (buf == NULL) {
        printf("malloc() failed.\n");
        exit(EXIT_FAILURE);
    }

    // 为iovec数组分配内存
    struct iovec* array = (struct iovec*)malloc(amount * sizeof(struct iovec));
    if (array == NULL) {
        printf("malloc() failed.\n");
        exit(EXIT_FAILURE);
    }

    // 初始化iovec数组的元素值
    for (int i = 0; i < amount; i++) {
        array->iov_base = buf;
        array->iov_len = length;
        buf += length;
    }

    return 0;
}












