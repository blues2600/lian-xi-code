/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* i6d_ucase_cl.c

   Client for i6d_ucase_sv.c: send each command-line argument as a datagram to
   the server, and then display the contents of the server's response datagram.
*/
#include "i6d_ucase.h"

int
main(int argc, char *argv[])
{
    // ipv6地址
    struct sockaddr_in6 svaddr;
    int sfd, j;
    size_t msgLen;
    ssize_t numBytes;
    char resp[BUF_SIZE];

    if (argc < 3 || strcmp(argv[1], "--help") == 0)
        usageErr("%s host-address msg...\n", argv[0]);

    /* Create a datagram socket; send to an address in the IPv6 domain */

    // 创建一个internet ipv6数据报套接字
    sfd = socket(AF_INET6, SOCK_DGRAM, 0);      /* Create client socket */
    if (sfd == -1)
        errExit("socket");

    // 地址置为0
    memset(&svaddr, 0, sizeof(struct sockaddr_in6));
    // internet ipv6
    svaddr.sin6_family = AF_INET6;
    // 端口从主机字节序转换为网络字节序
    svaddr.sin6_port = htons(PORT_NUM);

    // 将argv指向的字符串转换成网络字节序的二进制 IPv6 地址
    // 保存到svaddr中
    if (inet_pton(AF_INET6, argv[1], &svaddr.sin6_addr) <= 0)
        fatal("inet_pton failed for address '%s'", argv[1]);

    /* Send messages to server; echo responses on stdout */

    for (j = 2; j < argc; j++) {
        msgLen = strlen(argv[j]);
        // 利用sfd套接字向svaddr主机发送数据报
        // 等待发送的数据保存在argv中，长度为msglen字节
        if (sendto(sfd, argv[j], msgLen, 0, (struct sockaddr *) &svaddr,
                    sizeof(struct sockaddr_in6)) != msgLen)
            fatal("sendto");

        // 利用sfd套接字接收buf_size个字节的数据，保存到resp中
        // 发送者的地址信息丢弃
        numBytes = recvfrom(sfd, resp, BUF_SIZE, 0, NULL, NULL);
        if (numBytes == -1)
            errExit("recvfrom");

        printf("Response %d: %.*s\n", j - 1, (int) numBytes, resp);
    }

    exit(EXIT_SUCCESS);
}
