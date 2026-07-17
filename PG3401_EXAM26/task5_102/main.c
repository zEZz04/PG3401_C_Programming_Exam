/* MAIN.C - Task 5 ---------------------------------------------------------------------
 Project     : Exam PG3401 Spring 2026 - Task 5
 Author      : Candidate 102
 Description : Task 5 server entry point. Usage: task5 -port <port> -id <id>
---------------------------------------------------------------------------------------*/


/*=== Standard C library: ===============================================================
=======================================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


/*=== Project specific include files: ===================================================
=======================================================================================*/

#include "ewpdef.h"
#include "server.h"
#include "clienthandler.h"
#include "util.h"


/****************************************************************************************
*** MAIN FUNCTION
****************************************************************************************/

int main(int argc, char *argv[]) {
    int  iPort             = 0;
    int  iServerFd         = -1;
    int  iClientFd         = -1;
    int  iRunning          = 1;
    char szId[ID_SIZE]     = { 0 };

    /* Parse and validate -port and -id from command line */
    if(CheckCliInput(argc, argv, &iPort, szId, ID_SIZE) < 0) {
        fprintf(stderr, "- [ERROR] Invalid arguments.\n");
        fprintf(stderr, "Usage: task5 -port <port> -id <id>\n");
        return -1;
    }

    printf("- [INFO] Starting server id='%s' on port %d\n", szId, iPort);

    /* Create, bind and start listening on the given port */
    iServerFd = CreateServerSocket(iPort);
    if(iServerFd < 0) {
        fprintf(stderr, "- [ERROR] Could not create server socket.\n");
        return -1;
    }

    /* Accept and handle one client per iteration until QUIT shuts us down */
    while(iRunning) {
        printf("- [INFO] Waiting for client connection...\n");

        iClientFd = AcceptClient(iServerFd);
        if(iClientFd < 0) {
            fprintf(stderr, "- [WARNING] AcceptClient failed, retrying...\n");
            continue;
        }

        HandleClient(iClientFd, szId, &iRunning);

        close(iClientFd);
        iClientFd = -1;

        printf("- [INFO] Client session ended.\n");
    }

    close(iServerFd);
    iServerFd = -1;

    printf("- [INFO] Server shutting down. Goodbye.\n");
    return 0;
}

/*END OF FILE ----------------------------------------------------------------*/
