/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* id_echo_cl.c

   A client for the UDP "echo" service. This program sends each of its
   command-line arguments as a datagram to the server and echoes the
   contents of the datagrams that the server sends in response.

   See also id_echo_sv.c.
*/
#include "id_echo.h"

int
main(int argc, char *argv[])
{
    int sfd, j;
    size_t len;
    ssize_t numRead;
    char buf[BUF_SIZE];

    if (argc < 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s host msg...\n", argv[0]);

    /* Construct server address from first command-line argument */

    // 创建一个udp socket 并将其连接到argv[1]:service
    sfd = inetConnect(argv[1], SERVICE, SOCK_DGRAM);
    if (sfd == -1)
        fatal("Could not connect to server socket");

    /* Send remaining command-line arguments to server as separate datagrams */

    for (j = 2; j < argc; j++) {
        len = strlen(argv[j]);
        // 向sfd套接字发送最大长度为len的argv[x]缓冲区数据
        // 实际是向sfd连接的主机发送数据
        if (write(sfd, argv[j], len) != len)
            fatal("partial/failed write");

        // 从sfd套接字读取最多buf_size字节的数据保存到buf
        // sfd套接字的数据是由它连接的主机发送过来的
        numRead = read(sfd, buf, BUF_SIZE);
        if (numRead == -1)
            errExit("read");

        printf("[%ld bytes] %.*s\n", (long) numRead, (int) numRead, buf);
    }

    exit(EXIT_SUCCESS);
}
