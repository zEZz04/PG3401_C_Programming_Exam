/* MAIN.C - Task 6 ---------------------------------------------------------------------
 Project     : Exam PG3401 Spring 2026 - Task 6
 Author      : Candidate 102
 Description : Entry point for the EWA TCP over TCP network client.
         
---------------------------------------------------------------------------------------*/


/*=== Standard C library: ===============================================================
=======================================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*=== Project specific include files: ===================================================
=======================================================================================*/

#include "include/client.h"


/*=== PrintUsage: =======================================================================
=======================================================================================*/

static void PrintUsage(const char *prog) {
    fprintf(stderr, "Usage:   %s -server <ip> -port <port>\n", prog);
    fprintf(stderr, "Example: %s -server 127.0.0.1 -port 42420\n", prog);
}


/****************************************************************************************
*** MAIN FUNCTION
****************************************************************************************/

int main(int argc, char *argv[]) {
    const char *server_ip = NULL;
    const char *output    = "task6_received.bmp";
    int port              = 0;
    int i                 = 0;

    /* Parse command line arguments */
    for(i = 1; i < argc; i++) {
        if(strcmp(argv[i], "-server") == 0 && i + 1 < argc) {
            server_ip = argv[++i];
        } else if(strcmp(argv[i], "-port") == 0 && i + 1 < argc) {
            port = atoi(argv[++i]);
        } else {
            fprintf(stderr, "Unknown argument: %s\n", argv[i]);
            PrintUsage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    /* Validate that both required arguments were provided */
    if(!server_ip || port <= 0 || port > 65535) {
        PrintUsage(argv[0]);
        return EXIT_FAILURE;
    }

    return run_client(server_ip, port, output) == CLIENT_OK ? EXIT_SUCCESS : EXIT_FAILURE;
}

/*END OF FILE ----------------------------------------------------------------*/
