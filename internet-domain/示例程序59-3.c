/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* i6d_ucase_sv.c

   A server that receives datagrams, converts their contents to uppercase, and
   then returns them to the senders.

   See also i6d_ucase_cl.c.
*/
#include "i6d_ucase.h"

int
main(int argc, char *argv[])
{
    // ipv6 地址结构
    struct sockaddr_in6 svaddr, claddr;
    int sfd, j;
    ssize_t numBytes;
    socklen_t len;
    char buf[BUF_SIZE];
    char claddrStr[INET6_ADDRSTRLEN];

    /* Create a datagram socket bound to an address in the IPv6 domain */

    // 创建一个internet ipv6数据报套接字
    sfd = socket(AF_INET6, SOCK_DGRAM, 0);
    if (sfd == -1)
        errExit("socket");


    memset(&svaddr, 0, sizeof(struct sockaddr_in6));    //地址变量初始化为0
    svaddr.sin6_family = AF_INET6;                      //使用internet ipv6
    svaddr.sin6_addr = in6addr_any;                     //ipv6通配地址
    svaddr.sin6_port = htons(PORT_NUM);                 //先将端口转换为网络字节序再赋值

    if (bind(sfd, (struct sockaddr *) &svaddr,          //将套接字和地址绑定
                sizeof(struct sockaddr_in6)) == -1)
        errExit("bind");

    /* Receive messages, convert to uppercase, and return to client */

    for (;;) {
        len = sizeof(struct sockaddr_in6);

        // 数据报套接字就像邮箱
        // 从sfd接收buf_size字节的数据保存到buf，claddr保存信息发送者的地址
        numBytes = recvfrom(sfd, buf, BUF_SIZE, 0,
                            (struct sockaddr *) &claddr, &len);
        if (numBytes == -1)
            errExit("recvfrom");

        /* Display address of client that sent the message */

        // 将网络字节序二进制IP地址转换成字符串
        // 将claddr指向的ipv6的地址转换成字符串，保存到claddrstr中
        if (inet_ntop(AF_INET6, &claddr.sin6_addr, claddrStr,
                    INET6_ADDRSTRLEN) == NULL)
            printf("Couldn't convert client address to string\n");
        else
            printf("Server received %ld bytes from (%s, %u)\n",
                    (long) numBytes, claddrStr, ntohs(claddr.sin6_port));

        for (j = 0; j < numBytes; j++)
            buf[j] = toupper((unsigned char) buf[j]);

        // 利用sfd套接字，将buf中的numBytes字节数据以数据报的形式发送到claddr这个地址
        if (sendto(sfd, buf, numBytes, 0, (struct sockaddr *) &claddr, len) !=
                numBytes)
            fatal("sendto");
    }
}
