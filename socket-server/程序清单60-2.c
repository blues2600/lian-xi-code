/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* id_echo_sv.c

   This program implements a daemon that provides the UDP "echo" service. It
   reads datagrams and then sends copies back to the originating address.

   NOTE: this program must be run under a root login, in order to allow the
   "echo" port (7) to be bound. Alternatively, for test purposes, you can edit
   id_echo.h and replace the SERVICE name with a suitable unreserved port
   number (e.g., "51000"), and make a corresponding change in the client.

   See also id_echo_cl.c.
*/
#include <syslog.h>
#include "id_echo.h"
#include "become_daemon.h"

int
main(int argc, char *argv[])
{
    int sfd;
    ssize_t numRead;
    socklen_t len;
    struct sockaddr_storage claddr; //通用socket addr结构
    char buf[BUF_SIZE];
    char addrStr[IS_ADDR_STR_LEN];

    if (becomeDaemon(0) == -1)  //成为守护进程
        errExit("becomeDaemon");

    sfd = inetBind(SERVICE, SOCK_DGRAM, NULL); //创建internet domain通配地址service端口的数据报socket并绑定
    if (sfd == -1) {
        syslog(LOG_ERR, "Could not create server socket (%s)", strerror(errno));
        exit(EXIT_FAILURE);
    }

    /* Receive datagrams and return copies to senders */

    for (;;) {
        len = sizeof(struct sockaddr_storage);
        // 从sfd套接字接收数据报，大小最多BUF_SIZE，数据保存在buf中
        // 发送者的地址信息保存在claddr，地址存储的最大长度是len
        numRead = recvfrom(sfd, buf, BUF_SIZE, 0,
                           (struct sockaddr *) &claddr, &len);
        if (numRead == -1)
            errExit("recvfrom");

        // 向sfd套接字发送数据报，数据都在buf中，最多发送numread个字节
        // 接收者的地址保存在claddr中
        if (sendto(sfd, buf, numRead, 0, (struct sockaddr *) &claddr, len)
                        != numRead)
            syslog(LOG_WARNING, "Error echoing response to %s (%s)",
                    inetAddressStr((struct sockaddr *) &claddr, len,
                                   addrStr, IS_ADDR_STR_LEN),
                    strerror(errno));
    }
}
