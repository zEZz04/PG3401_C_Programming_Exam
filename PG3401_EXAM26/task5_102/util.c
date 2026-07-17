/* UTIL.C - Task 5 ---------------------------------------------------------------------
 Project     : Exam PG3401 Spring 2026 - Task 5
 Author      : Candidate 102
 Description : Utility and helper functions for CLI parsing and string building
---------------------------------------------------------------------------------------*/


/*=== Standard C library: ===============================================================
=======================================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


/*=== Project specific include files: ===================================================
=======================================================================================*/

#include "ewpdef.h"
#include "util.h"


/*=== IsValidInteger: ===================================================================
=======================================================================================*/

int IsValidInteger(char *str) {
    int i = 0;

    if(str == NULL || str[0] == '\0') return 0;

    for(i = 0; str[i] != '\0'; i++) {
        if(str[i] < '0' || str[i] > '9') return 0;
    }

    return 1;
}


/*=== CheckCliInput: ====================================================================
=======================================================================================*/

int CheckCliInput(int argc, char *argv[], int *piPort, char *szId, int iIdSize) {
    int i = 0;

    if(argc < 5) {
        fprintf(stderr, "- [ERROR] Too few arguments.\n");
        return -1;
    }

    for(i = 1; i < argc - 1; i++) {
    /* Check each argument pair, argv[i] is the flag and argv[i+1] is the value */
        if(strcmp(argv[i], "-port") == 0) {
            if(!IsValidInteger(argv[i + 1])) {
                fprintf(stderr, "- [ERROR] Port must be a positive integer.\n");
                return -1;
            }
            *piPort = atoi(argv[i + 1]);
        } else if(strcmp(argv[i], "-id") == 0) {
            strncpy(szId, argv[i + 1], iIdSize - 1);
            szId[iIdSize - 1] = '\0';
        }
    }

    if(*piPort <= 0 || szId[0] == '\0') {
        fprintf(stderr, "- [ERROR] Both -port and -id has to be provided.\n");
        return -1;
    }

    return 0;
}


/*=== MakeFormattedAcceptString: ========================================================
=======================================================================================*/

int MakeFormattedAcceptString(char *buffer, char *szServerId) {
    time_t      tNow        = 0;
    struct tm  *pTm         = NULL;
    char        szTime[32]  = { 0 };
    char        szBuf[80]   = { 0 };

    if(buffer == NULL || szServerId == NULL) return -1;

    tNow = time(NULL);
    pTm  = localtime(&tNow);

    strftime(szTime, sizeof(szTime), "%d %b %Y, %H:%M:%S", pTm);
    sprintf(szBuf, "127.0.0.1 SMTP %s %s", szServerId, szTime);

    memcpy(buffer, szBuf, 49);
    buffer[49] = '\0';

    return 0;
}

/*END OF FILE ----------------------------------------------------------------*/

