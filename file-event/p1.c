/*
 * 编写一个程序，针对其命令行参数所指定的目录，记录所有的文件创建、删除和改
 * 名操作。该程序应能够监控指定目录下所有子目录中的事件。获得所有子目录的列
 * 表需使用 nftw()（参见 18.9 节）。当在目录树下添加或删除了子目录时，受监控的
 * 子目录集合应能保持同步更新。
 */

/*
 * 任务解析
 * 1.监控目录，范围包含目录中所有子目录
 * 2.监控类型：创建、删除、改名
 * 3.子目录修改后，监控范围同步修改
 */

/*
 * 思考
 * 总结inotify机制的行为
 * 关注nftw()与监控机制的结合
 * 考虑任务解析的第3点
 */

/*
 * inotify机制的原理和行为
 * 1.新建监控实例inotify_init()
 * 2.通知监控实例，要监控什么对象(char *)，以及监控该对象的哪些行为inotify_add_watch()
 * 3.从监控实例读取时间消息（如果有的话）
 * 4.打印消息
 */

/*
 * 监控整个目录树,nftw()与监控机制的结合
 * - 要监控整个目录树就要利用nftw(),为什么？
 * - 因为nftw遍历dirpath路径下包含的所有文件，它会获得文件的路径、文件的详细信息、目录深度、文件类型等等
 * - 而且它还对每个文件都执行一次fn函数
 * - 那么，有了文件路径，以及可以对文件执行一次fn函数，就可以在这里通知监控示例，要监控什么对象，以及监控它的哪些行为
 *
 */

/*
 * 总结：
 * 1.这个程序要入门，就得先理解inotify机制，这个相对容易
 * 2.比较繁琐或者有难度的是对inotify机制细节的理解，要深入理解inotify机制会很费时间
 * 3.本程序的bug基本出现在对数据的操作上，对于链表的把握还比较欠缺
 * 4.对自己目前的调试和拍错能力感到满意
 */

#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <limits.h>
#include <ftw.h>
#include <sys/inotify.h>
#include <unistd.h>

//limits.h里面的PATH_MAX太大了
#define MY_PATH_MAX 512
#define BUF_LEN (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))

// 路径-监控描述符表
typedef struct path_watchfd_table {
    struct path_watchfd_table *previous;
    char path[MY_PATH_MAX];
    int watchfd;
    struct path_watchfd_table *next;
} PathWatchTable;

// 全局变量
int inotify_fd = -1;           //监控程序实例的文件描述符
PathWatchTable *first = NULL;  //指向路径-监控描述符表的第一个条目
PathWatchTable *end = NULL;  //指向路径-监控描述符表的最后一个条目

// 新建一个inotify监控实例
// 对inotify_init()的封装
// 成功返回监控实例程序描述符，失败返回-1并退出程序
int MyInotifyInit()
{
    int fd = -1;

    fd = inotify_init();
    if (fd == -1) {
        printf("inotify_init() failed, %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    return fd;
}

// 判断一个文件是否为目录
bool MyIsDir(char *path)
{
    struct stat buf;

    stat(path, &buf);
    if (S_ISDIR(buf.st_mode))
        return true;
    else
        return false;
}

// 添加一个监控对象
// 成功返回监控描述符，失败退出程序，并输出错误提示
int MyInotifyAdd(int fd, const char *path, uint32_t mask)
{
    int watch_fd = -1;

    watch_fd = inotify_add_watch(fd, path, mask);
    if (watch_fd == -1) {
        printf("inotify_add_watch() failed, %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    return watch_fd;
}

// 分配一个PathWatchTable类型大小的堆内存
// 堆malloc的简单封装
// 成功返回新内存指针，失败退出程序，并输出错误提示
PathWatchTable *MyMalloc()
{
    PathWatchTable *new = NULL;

    new = (PathWatchTable *)malloc(sizeof(PathWatchTable));
    if (new == NULL) {
        printf("malloc() failed in MyMalloc().\n");
        exit(EXIT_FAILURE);
    }

    return new;
}

// nftw函数的fn函数
int WatchAdd(const char *fpath, const struct stat *sb, int typeflag,
             struct FTW *ftwbuf)
{
    int fd = -1;
    // 将当前文件纳入监控范围
    // 监控类型：新增、删除、改名，且不对符号链接解开引用
    fd = MyInotifyAdd(inotify_fd, fpath,
                      IN_DONT_FOLLOW | IN_CREATE | IN_DELETE_SELF |
                          IN_MOVE_SELF);

    // 在路径-描述符表中注册当前文件
    PathWatchTable *new = MyMalloc();
    if (first == NULL) {  //判断是否为表中的第一个条目

        // 初始化条目内容
        new->previous = NULL;  //第一个条目没有上一个条目可以指向
        new->next = NULL;      //暂时也没有下一个条目
        strcpy(new->path, fpath);  //在表中初始化文件路径
        new->watchfd = fd;         //在表中初始化监控描述符

        // 重设链表
        first = new;  // 因为是第一个条目，所以理所当然
        end = new;    //因为是第一个条目，所以end也是自己
        //current = new;
    } else {  //不是表中的第一个条目
        // 初始化条目内容
        end->next = new;
        new->previous = end;
        new->next = NULL;
        strcpy(new->path, fpath);
        new->watchfd = fd;

        // 重设链表
        end = new;      //末尾条目更新
    }

    printf("[+] %s just created, already added to the monitoring list\n",
           fpath);
    return 0;  //返回非零值，则树遍历终止
}

// 从监控实例程序中读取监控事件消息
// 成功返回读取的字节数，失败返回0，并输出错误消息
int ReadEventMsg(int fd, char *buf, size_t len)
{
    ssize_t num = -1;

    num = read(fd, buf, len);
    if (num == -1) {
        printf("read() failed, %s\n", strerror(errno));
        return 0;
    }
    if (num == 0) {
        printf("it's EOF, from read() in ReadEventMsg(). \n");
        return 0;
    }

    return num;
}

// 查询路径-描述符表，根据fd(wd)查询表的条目
// 成功返回条目指针，失败返回NULL，并输出错误信息
PathWatchTable *TableQuery(int fd)
{
    PathWatchTable *temp = NULL;
    PathWatchTable *p = NULL;
    temp = first;  //指向链表第一个条目

    while (1) {
        if (temp->watchfd == fd)
            return temp;
        if (temp == end) {  //判断链表结尾
            printf("[*]error: cann't find the target watchfd %d.\n", fd);
            return NULL;
        }
        p = temp->next;  //指向链表下一个节点
        if(p == NULL)
        {
            printf("end=%p\n",end);
            printf("temp=%p\n",temp);
            printf("watchfd=%d\n",temp->watchfd);
            printf("temp=%s\n",temp->path);
            exit(EXIT_FAILURE);
        }
        temp = p;
    }
}

// 打印inotify_event结构体的数据
int PrintInotifyEventData(struct inotify_event *p)
{
    PathWatchTable *element = NULL;

    printf("[*] watch id %d,", p->wd);
    printf("cookie id %d,", p->cookie);
    printf("event type ");

    // 判断事件类别.
    if (p->mask & IN_CREATE)
        printf("IN_CREATE,");
    if (p->mask & IN_DELETE_SELF)
        printf("IN_DELETE_SELF,");
    if (p->mask & IN_MOVE_SELF)
        printf("IN_MOVE_SELF(rename),");
    if (p->mask & IN_IGNORED) {
        printf("IN_IGNORED, file has been deleted\n");
        return 0;
    }

    // 当前被监控文件发生内容更新时，也会产生IN_CREATE事件

    // 查询路径-描述符表，输出事件关联的文件路径
    element = TableQuery(p->wd);
    if (element)
        printf("file:%s\t", element->path);

    // 打印消息本身
    if (p->len > 0)
        printf("msg:%s\n", p->name);
    else {
        fflush(stdout);
        printf("\n");
    }
    return 0;
}

// 删除链表的一个条目(路径-描述符表
// 提供元素的watchfd即可
void DeleteElement(int watchfd)
{
    PathWatchTable *temp = NULL;
    PathWatchTable *element = NULL;

    element = TableQuery(watchfd);  //通过fd获得条目指针

    // 判断是不是链表的第一个条目
    if (element == first) {
        if(element->next == NULL)   //链表只有一个条目
            goto FREE;
        temp = element->next;   //temp指向下一个条目
        temp->previous = NULL;  //下一个条目成为链表第一个条目
        first = temp;           //重置first
        goto FREE;
    }
    // 判断是不是链表的最后一个条目
    if (element == end) {
        temp = element->previous;  //temp指向上一个条目
        temp->next = NULL;         //清空链表末尾的指针
        end = temp;                //重置end
        goto FREE;
    }
    // 链表中间的条目
    temp = element->previous;            //temp指向前一个条目
    temp->next = element->next;          //上个条目与下个条目链接
    temp = element->next;                //temp指向下一个条目
    temp->previous = element->previous;  //下个条目和上个条目链接

FREE:
    free(element);
}

// 监控事件处理函数 - 文件删除
void EventProcessDel(struct inotify_event *event)
{
    /*
     * 在本机测试，当被监控的文件删除后不需要显式调用inotify_rm_watch
     * 内核会自动删除文件的监控项，同时产生IN_IGNORED事件
     * 如果在IN_DELETE_SELF事件发生后显式调用inotify_rm_watch
     * 则函数返回-1，并报告EINVAL错误
     *
     * 虽然函数只有一条语句，但仍然决定保留，以备今后参考
     */

    /*
     * 以下代码不再需要
    // 移除监控范围
    retval = inotify_rm_watch(inotify_fd, event->wd);
    if (retval == -1) {
        printf("inotify_rm_watch() failed in EventProcessDel(), %s\n",
               strerror(errno));
        //exit(EXIT_FAILURE);
    }*/

    // 将条目从链表删除
    DeleteElement(event->wd);
}

// 在一个绝对路径中取出最后一个文件名
int GetNameFromPath(char *path, char *result)
{
    char limt[] = "/";

    char *token = strtok(path, limt);
    if (token == NULL) {
        printf("strtok() failed. \n");
        return 0;
    }

    while (token != NULL) {
        strcpy(result, token);
        token = strtok(NULL, limt);
    }
    return 1;
}

// 监控事件处理函数 - 目录删除
void EventProcessDelDir(struct inotify_event *event)
{
    PathWatchTable *element = NULL;
    PathWatchTable *list = first;
    char dirname[NAME_MAX] = {'\0'};
    char temp[] = "/";

    element = TableQuery(event->wd);          //获得条目指针
    GetNameFromPath(element->path, dirname);  //取出目录名

    // 目录名连接成/dir/形式，用于在路径中搜索该目录下的文件
    strcat(temp, dirname);
    strcat(dirname, "/");

    EventProcessDel(event);  //先处理目录本身

    // 搜索路径-描述符表，所有归属于dirname目录下的文件都移除监控范围，并删除表条目
    while (1) {
        if (strstr(list->path, dirname))  //是否属于该目录下的文件
            EventProcessDel(event);       //是，处理文件

        if (list == end)
            break;

        list = list->next;
    }
}

// 监控事件处理函数 - 文件添加
void EventProcessAdd(struct inotify_event *event)
{
    char newfile[MY_PATH_MAX] = {'\0'};

    // 查询文件添加事件发生的路径
    PathWatchTable *element = TableQuery(event->wd);

    // 生成新添加的文件绝对路径
    strcat(newfile, element->path);
    strcat(newfile, "/");
    if (event->len > 0)
        strcat(newfile, event->name);
    else
        printf("\nerror:the file name is null\n");

    // 新文件加入监控并注册路径-描述符表
    WatchAdd(newfile, NULL, 0, NULL);
}

// 监控事件处理函数
// 例如当发生文件删除事件时，对应的监控范围和路径描述符表都会被删除
void EventProcess(struct inotify_event *event)
{
    PathWatchTable *element = NULL;

    // 判断事件类别
    if (event->mask & IN_CREATE)  //新建事件，不需要区分文件还是目录
        EventProcessAdd(event);

    if (event->mask & IN_DELETE_SELF) {   //删除事件
        element = TableQuery(event->wd);  //查询路径-描述符表
        if (MyIsDir(element->path))       //是否为目录
            EventProcessDelDir(event);
        else
            EventProcessDel(event);
    }

    if (event->mask & IN_MOVE_SELF)  //文件重命名
        printf(
            "[*] The file has been renamed, but the current function cannot track the new name of the file (event->name cannot get the new file name)\n");
}

// 输出监控事件消息
void DisplayEventMsg(char *msg, ssize_t len)
{
    char *ptr = NULL;
    struct inotify_event *event = NULL;

    for (ptr = msg; ptr < msg + len;) {       // 限定指针在msg范围内
        event = (struct inotify_event *)ptr;  // 抽取inotify_event结构数据
        PrintInotifyEventData(event);         // 打印事件消息
        EventProcess(event);                  // 根据消息内容处理事件
        ptr += sizeof(struct inotify_event) + event->len;  //指向下一条事件消息
    }
}

int main(int argc, char *argv[])
{
    int retval = 0;
    ssize_t numRead = -1;
    char msg[BUF_LEN] = {'\0'};

    if (argc != 2) {
        printf("useage: ./app directory\n");
        printf("The directory must be a full path.\n");
        exit(EXIT_FAILURE);
    }

    // 判断监控目标是否为目录
    if (!MyIsDir(argv[1])) {
        printf("the %s not a directory.\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    // 新建inotify实例
    inotify_fd = MyInotifyInit();

    // 遍历要监控的目录
    // 对遍历的每个文件都执行WatchAdd()，添加到监控范围
    // 不会对目录中的符号链接解开引用，这意味着只监控符号链接本身
    retval = nftw(argv[1], WatchAdd, 15, FTW_PHYS);
    if (retval != 0) {
        printf("nftw() failed, %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // 从监控程序实例读取事件消息
    for (;;) {
        numRead = ReadEventMsg(inotify_fd, msg, BUF_LEN);  //读取
        if (numRead != 0)                                  //若读取ok
            DisplayEventMsg(msg,numRead);  //输出监控事件消息并根据事件内容做出反应
    }

    return 0;
}
