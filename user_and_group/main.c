/*
 * 任务
 * 使用 setpwent()、getpwent()和 endpwent()来实现 getpwnam()
 *
 */

/*
 *
 * struct passwd *getpwnam(const char * name );
 * getpwnam()的功能：从passwd文件获取记录，给name一个登录名，函数会返回一个struct passwd指针
 * 如果没有发现name匹配记录，函数返回NULL
 *
 */

#define _POSIX_C_SOURCE
#define _DEFAULT_SOURCE
#define _XOPEN_SOURCE_EXTENDED

#include <sys/types.h> /* 许多类型的定义 */
#include <stdio.h>     /* Standard I/O functions */
#include <stdlib.h>    /* 常用库函数的原型 */
#include <unistd.h>    /* 许多系统调用的原型 */
#include <errno.h>     /* 声明 errno 和定义错误常量 */
#include <string.h>    /* Commonly used string-handling functions */
#include <pwd.h>

#define min(m, n) ((m) < (n) ? (m) : (n))
#define max(m, n) ((m) > (n) ? (m) : (n))

// 从passwd文件获取记录
// 给name一个登录名，函数会返回一个struct passwd指针
// 如果没有发现name匹配记录，函数返回NULL
struct passwd* my_getpwnam(char *name)
{
    struct passwd *pwd;

    // 循环遍历密码数据库，每次循环得到一条记录
    while ((pwd = getpwent()) != NULL) {
        if (strcmp(pwd->pw_name, name) == 0) {
            endpwent();  // 关闭密码数据库
            return pwd;
        }
    }

    // 没有找到对应用户名的记录
    endpwent();
    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("useage: filename username\n");
        exit(EXIT_FAILURE);
    }

    struct passwd *pwd = NULL;

    pwd = my_getpwnam(argv[1]);
    if (pwd != NULL)
        printf("the %s uid=%u gid=%u\n", pwd->pw_name, pwd->pw_uid, pwd->pw_gid);
    else
        printf("can not find user in database.\n");

    return 0;
}
