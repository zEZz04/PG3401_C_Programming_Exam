/* SERVER.C - Task 5 -------------------------------------------------------------------
 Project     : Exam PG3401 Spring 2026 - Task 5
 Author      : Candidate 102
 Description : Server socket creation, binding and client accept logic
---------------------------------------------------------------------------------------*/


/*=== Standard C library: ===============================================================
=======================================================================================*/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>


/*=== Socket include files: =============================================================
=======================================================================================*/

#include <arpa/inet.h>
#include <sys/socket.h>


/*=== Project specific include files: ===================================================
=======================================================================================*/

#include "ewpdef.h"
#include "server.h"


/*=== CreateServerSocket: ===============================================================
=======================================================================================*/

int CreateServerSocket(int iPort) {
    int iSockFd             = -1;
    int iOptVal             = 1;
    struct sockaddr_in saAddr;
    memset(&saAddr, 0, sizeof(saAddr));

    iSockFd = socket(AF_INET, SOCK_STREAM, 0);
    if(iSockFd < 0) {
        fprintf(stderr, "- [ERROR] socket() failed, errno=%d\n", errno);
        return -1;
    }

    /* Allow quick port reuse after a restart so we don't get address-in-use errors */
    if(setsockopt(iSockFd, SOL_SOCKET, SO_REUSEADDR, &iOptVal, sizeof(iOptVal)) < 0) {
        fprintf(stderr, "- [ERROR] setsockopt() failed, errno=%d\n", errno);
        close(iSockFd);
        return -1;
    }

    saAddr.sin_family      = AF_INET;
    saAddr.sin_port        = htons(iPort);
    saAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if(bind(iSockFd, (struct sockaddr *)&saAddr, sizeof(saAddr)) < 0) {
        fprintf(stderr, "- [ERROR] bind() failed on port %d, errno=%d\n", iPort, errno);
        close(iSockFd);
        return -1;
    }

    if(listen(iSockFd, MAX_CLIENTS) < 0) {
        fprintf(stderr, "- [ERROR] listen() failed, errno=%d\n", errno);
        close(iSockFd);
        return -1;
    }

    printf("- [INFO] Server listening on 127.0.0.1:%d\n", iPort);
    return iSockFd;
}


/*=== AcceptClient: =====================================================================
=======================================================================================*/

int AcceptClient(int iServerSocket) {
    int iClientFd           = -1;
    int iAddrLen            = sizeof(struct sockaddr_in);
    struct sockaddr_in saClient;
    memset(&saClient, 0, sizeof(saClient));

    iClientFd = accept(iServerSocket,
                       (struct sockaddr *)&saClient,
                       (socklen_t *)&iAddrLen);
    if(iClientFd < 0) {
        fprintf(stderr, "- [ERROR] accept() failed, errno=%d\n", errno);
        return -1;
    }

    printf("- [INFO] Connection accepted from %s\n", inet_ntoa(saClient.sin_addr));
    return iClientFd;
}

/*END OF FILE ----------------------------------------------------------------*/

