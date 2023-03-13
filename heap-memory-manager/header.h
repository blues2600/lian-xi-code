
#include <sys/types.h>  /* 许多类型的定义 */
#include <stdio.h>      /* Standard I/O functions */
#include <stdlib.h>     /* 常用库函数的原型 */
#include <unistd.h>     /* 许多系统调用的原型 */
#include <errno.h>      /* 声明 errno 和定义错误常量 */
#include <string.h>     /* Commonly used string-handling functions */

#define min(m,n) ((m) < (n) ? (m) : (n))
#define max(m,n) ((m) > (n) ? (m) : (n))
