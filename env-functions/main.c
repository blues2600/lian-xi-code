/*
 * 使用 getenv()函数、putenv()函数，必要时可直接修改 environ，来实现 setenv()函数
 * 和 unsetenv()函数。此处的 unsetenv()函数应检查是否对环境变量进行了多次定义，
 * 如果是多次定义则将移除对该变量的所有定义（glibc 版本的 unsetenv()函数实现了这一功能）。
 */

/*
 * 任务分解
 * 1.先写setenv
 * 2.再考虑unsetenv
 */

/* 任务1，mysetenv，设置或修改环境变量
 */

//#define _BSD_SOURCE
//#define _GNU_SOURCE
//#define _SVID_SOURCE
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

extern char **environ;
int mySetenv(const char *name, const char *value, int overwrite);

char name_and_value[255];

int main(int argc, char *argv[])
{
    const char *name = "TEST";
    const char *value = "456";
    if (!mySetenv(name, value, 1))
        printf("the env is:%s\n",getenv(name));
    return 0;
}

// 变量名称添加到具有值value的环境中
// 如果 name确实存在于环境中，那么如果overwrite不为零，则将其值更改为value
// 如果overwrite为零，则名称的值不会更改
// 函数复制name和value指向的字符串
// 成功返回零，失败返回 -1，并设置errno以指示错误原因
int mySetenv(const char *name, const char *value, int overwrite)
{
    // 连接name和value
    if (!strcat(name_and_value, name))
        printf("strcat() failed\n");
    if (!strcat(name_and_value, "="))
        printf("strcat() failed\n");
    if (!strcat(name_and_value, value))
        printf("strcat() failed\n");

    // 查找环境变量
    // 环境变量不存在，添加一个新的
    if (getenv(name) == NULL) {
        if (putenv(name_and_value) != 0) {
            printf("putenv() failed, %s\n", strerror(errno));
            return -1;
        }
        goto SUCCESS;
    }

    // 环境变量存在，且overwrite不为0，修改现有环境变量
    if (overwrite != 0) {
        if (putenv(name_and_value) != 0) {
            printf("putenv() failed, %s\n", strerror(errno));
            return -1;
        }
        goto SUCCESS;
    }

SUCCESS:
    return 0;
}
