/*
 *
 * 在程序清单 42-1 中的程序（dynload.c）中添加一个 dladdr()调用以获取与 dlsym()
 * 返回的地址有关的信息。打印出返回的 Dl_info 结构中各个字段的值并验证这些值
 * 是否与预期的值一样。
 *
 */

// 运行程序 LD_LIBRARY_PATH=. ./app ./liblu.so hello

#define _GNU_SOURCE

#include <dlfcn.h>
#include <stdio.h>

#include "mylib.h"

int main(int argc, char **argv) {
    void *libHandle;
    void (*funcp)(void);
    const char *err;
    Dl_info symbol;

    if (argc != 3)
        printf("%s lib-path func-name\n", argv[0]);

    libHandle = dlopen(argv[1], RTLD_LAZY);
    if (libHandle == NULL) printf("dlopen():%s\n", dlerror());

    // 清空之前的错误
    (void)dlerror();
    funcp = (void (*)(void))dlsym(libHandle, argv[2]);

    err = dlerror();
    if (err != NULL) printf("dlsym():%s\n", dlerror());

    (*funcp)();

    // 获取符号信息并打印
    if(!dladdr(argv[2], &symbol)){
        err = dlerror();
        if (err != NULL) printf("dlsym():%s\n", dlerror());
    }

    printf("lib path=%s\n", symbol.dli_fname);
    printf("symbol module base=%p\n", (unsigned long long int *)symbol.dli_fbase);
    printf("symbol name=%s\n", symbol.dli_sname);
    printf("symbol addr=%p\n", (unsigned long long int *)symbol.dli_saddr);

    dlclose(libHandle);

    return 0;
}

