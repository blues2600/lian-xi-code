/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* is_seqnum_sv.c

   A simple Internet stream socket server. Our service is to provide
   unique sequence numbers to clients.

   Usage:  is_seqnum_sv [init-seq-num]
                        (default = 0)

   See also is_seqnum_cl.c.
*/
#define _BSD_SOURCE /* To get definitions of NI_MAXHOST and \
                       NI_MAXSERV from <netdb.h> */
#include <netdb.h>

#include "is_seqnum.h"

#define BACKLOG 50

int main(int argc, char *argv[]) {
    uint32_t seqNum;
    char reqLenStr[INT_LEN]; /* Length of requested sequence */
    char seqNumStr[INT_LEN]; /* Start of granted sequence */

    // 万能套接字地址存储
    struct sockaddr_storage claddr;
    int lfd, cfd, optval, reqLen;
    socklen_t addrlen;

    struct addrinfo hints;
    struct addrinfo *result, *rp;

    // 主机和服务名字符串最大限制的和
#define ADDRSTRLEN (NI_MAXHOST + NI_MAXSERV + 10)
    char addrStr[ADDRSTRLEN];
    char host[NI_MAXHOST];
    char service[NI_MAXSERV];

    if (argc > 1 && strcmp(argv[1], "--help") == 0)
        usageErr("%s [init-seq-num]\n", argv[0]);

    seqNum = (argc > 1) ? getInt(argv[1], 0, "init-seq-num") : 0;

    /* Ignore the SIGPIPE signal, so that we find out about broken connection
       errors via a failure from write(). */

    // 忽略 SIGPIPE 信号，以便我们通过 write() 的失败找出连接中断错误。
    // 忽略 SIGPIPE 信号②。这样就能够防止服务器在尝试向一个对端已经被关闭的
    // socket 写入数据时收到 SIGPIPE 信号
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) errExit("signal");

    /* Call getaddrinfo() to obtain a list of addresses that
       we can try binding to */

    // 指定getaddrinfo返回的result链表中的数据格式
    // 套接字类型为流
    // 可以为ipv6 or ipv6
    // 返回通配符ip地址
    // service必须指向包含数字端口号的字符串
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC; /* Allows IPv4 or IPv6 */
    hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;
    /* Wildcard IP address; service name is numeric */

    // 将主机和服务名转换成 IP 地址和端口号
    // 这是是将端口对应的socket地址提取出来
    if (getaddrinfo(NULL, PORT_NUM, &hints, &result) != 0)
        errExit("getaddrinfo");

    /* Walk through returned list until we find an address structure
       that can be used to successfully create and bind a socket */

    optval = 1;
    // rp指向返回的链表头
    // 循环访问getaddrinfo返回的ip地址和端口号数据
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        // 创建套接字
        lfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (lfd == -1) continue; /* On error, try next address */

        // 端口释放后立即就可以被再次使用
        if (setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &optval,
                       sizeof(optval)) == -1)
            errExit("setsockopt");

        // 把套接字绑定到地址
        // 成功绑定就会跳出去
        if (bind(lfd, rp->ai_addr, rp->ai_addrlen) == 0) break; /* Success */

        /* bind() failed: close this socket and try next address */

        close(lfd);
    }

    if (rp == NULL) fatal("Could not bind socket to any address");

    if (listen(lfd, BACKLOG) == -1) errExit("listen");

    freeaddrinfo(result);

    for (;;) { /* Handle clients iteratively */

        /* Accept a client connection, obtaining client's address */

        addrlen = sizeof(struct sockaddr_storage);
        // 从lfd接受一个连接请求，和claddr地址创建新的socket连接
        cfd = accept(lfd, (struct sockaddr *)&claddr, &addrlen);
        if (cfd == -1) {
            errMsg("accept");
            continue;
        }

        // 将claddr的地址主数据翻译成主机和服务名，保存到host和service中
        if (getnameinfo((struct sockaddr *)&claddr, addrlen, host, NI_MAXHOST,
                        service, NI_MAXSERV, 0) == 0)
            // host和service写到addrstr里
            snprintf(addrStr, ADDRSTRLEN, "(%s, %s)", host, service);
        else
            snprintf(addrStr, ADDRSTRLEN, "(?UNKNOWN?)");
        printf("Connection from %s\n", addrStr);

        /* Read client request, send sequence number back */

        // 从cfd套接字最多读取int_len个字节数据到reqlenstr
        if (readLine(cfd, reqLenStr, INT_LEN) <= 0) {
            close(cfd);
            continue; /* Failed read; skip request */
        }

        reqLen = atoi(reqLenStr);
        if (reqLen <= 0) { /* Watch for misbehaving clients */
            close(cfd);
            continue; /* Bad request; skip it */
        }

        snprintf(seqNumStr, INT_LEN, "%d\n", seqNum);
        if (write(cfd, seqNumStr, strlen(seqNumStr)) != strlen(seqNumStr))
            fprintf(stderr, "Error on write");

        seqNum += reqLen; /* Update sequence number */

        if (close(cfd) == -1) /* Close connection */
            errMsg("close");
    }
}
