/* TASK3_UTIL.C - Task 3 ---------------------------------------------------------------
 Project     : Exam PG3401 Spring 2026 - Task 3
 Author      : Candidate 102
 Description : Utility functions for safe input, whitespace handling and validation
---------------------------------------------------------------------------------------*/


/*=== Standard C library: ===============================================================
=======================================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


/*=== Project specific include files: ===================================================
=======================================================================================*/

#include "task3_util.h"


/*=== StripWhitespace: ==================================================================
=======================================================================================*/

void StripWhitespace(char *szText) {
    int iStart = 0;
    int iEnd;
    int i;      

    if(szText == NULL || szText[0] == '\0') return;

    iEnd = (int)strlen(szText) - 1;

    /* Trim trailing whitespace */
    while(iEnd >= 0 && (szText[iEnd] == ' '  || szText[iEnd] == '\n'
                     || szText[iEnd] == '\r'  || szText[iEnd] == '\t')) {
        szText[iEnd] = '\0';
        iEnd--;
    }

    /* Find first non-whitespace character */
    while(szText[iStart] == ' '  || szText[iStart] == '\n'
       || szText[iStart] == '\r' || szText[iStart] == '\t') {
        iStart++;
    }

    /* Shift text left to remove leading whitespace */
    if(iStart > 0) {
        i = 0;
        while(szText[iStart] != '\0') {
            szText[i] = szText[iStart];
            iStart++;
            i++;
        }
        szText[i] = '\0';
    }
}


/*=== GetStringInput: ===================================================================
=======================================================================================*/

void GetStringInput(char *pszBuffer, int iMaxLength) {
    int i = 0;
    int c = 0;

    while(i < iMaxLength - 1) {
        c = getchar();
        if(c == '\n' || c == EOF) break;
        pszBuffer[i++] = (char)c;
    }
    pszBuffer[i] = '\0';

    /* Flush stdin if the user typed more than the buffer can hold */
    if(c != '\n' && c != EOF) {
        ClearStdin();
    }

    StripWhitespace(pszBuffer);
}


/*=== GetIntegerInput: ==================================================================
=======================================================================================*/

int GetIntegerInput(char *prompt, int *piBuffer) {
    char szBuffer[32] = { 0 };

    while(1) {
        printf("%s", prompt);

        if(fgets(szBuffer, sizeof(szBuffer), stdin) == NULL) {
            printf("Input failed to read. Try again.\n");
            continue;
        }

        /* If newline is missing, the input was longer than the buffer, flush it */
        if(strchr(szBuffer, '\n') == NULL) {
            ClearStdin();
        } else {
            szBuffer[strcspn(szBuffer, "\n")] = '\0';
        }

        if(sscanf(szBuffer, "%d", piBuffer) == 1) {
            return 1;
        } else {
            printf("Not a valid number. Try again.\n");
        }
    }
}


/*=== ClearStdin: =======================================================================
=======================================================================================*/

void ClearStdin(void) {
    int c = 0;
    while((c = getchar()) != '\n' && c != EOF);
}


/*=== IsValidCandidateID: ===============================================================
=======================================================================================*/

int IsValidCandidateID(char szCandidateID[]) {
    if(szCandidateID[0] == '\0') {
        printf("Error: Candidate ID cannot be empty.\n");
        return 0;
    }
    return 1;
}


/*=== IsValidJustification: =============================================================
=======================================================================================*/

int IsValidJustification(char justification[]) {
    int i;

    if(justification[0] == '\0') {
        printf("Error: Justification cannot be empty.\n");
        return 0;
    }

    for(i = 0; justification[i] != '\0'; i++) {
        if(!isspace((unsigned char)justification[i])) {
            return 1;
        }
    }

    printf("Error: Justification cannot contain only spaces.\n");
    return 0;
}


/*=== CalculateGrade: ===================================================================
=======================================================================================*/

char CalculateGrade(int finalScore) {
    if(finalScore >= 90)      return 'A';
    if(finalScore >= 77)      return 'B';
    if(finalScore >= 64)      return 'C';
    if(finalScore >= 51)      return 'D';
    if(finalScore >= 41)      return 'E';
    return 'F';
}

/*END OF FILE ----------------------------------------------------------------*/

