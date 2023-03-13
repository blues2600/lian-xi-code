#define _GNU_SOURCE     /* 需要定义该符号，从 <stdlib.h> 获取各种声明 */
#include <stdlib.h>

// 在 C 语言程序中，可以使用全局变量 char **environ 访问环境列表。
// C 运行时启动代码定义了该变量并以环境列表位置为其赋值
extern char **environ;
int
main(int argc, char *argv[])
{
    int j;
    char **ep;

    // 清除整个环境列表，但是 SUSv3 没有对此函数进行规范
    // clearenv()会泄露setenv()函数所分配的内存
    // SUSv3 允许应用程序清空自身环境的唯一方法是首先获取所有环境变量的列表（通过
    // environ 变量获得所有环境变量的名称），然后逐一调用 unsetenv()移除每个环境变量
    clearenv();         /* Erase entire environment */

    // putenv()函数向调用进程的环境中添加一个新变量，或者修改一个已经存在的变量值
    // putenv()把参数的地址给了environ变量，所以参数不应该设置为自动变量
    for (j = 1; j < argc; j++)
        if (putenv(argv[j]) != 0)
            errExit("putenv: %s", argv[j]);

    // 设置一个新的环境变量，参数可以为自动变量，且第三个参数可以根据情况来决定覆盖原有变量
    if (setenv("GREET", "Hello world", 0) == -1)
        errExit("setenv");

    // 对应于setenv
    unsetenv("BYE");

    // 显示当前所有环境变量的值
    for (ep = environ; *ep != NULL; ep++)
        puts(*ep);

    exit(EXIT_SUCCESS);
}
