/* TASK3_MAIN.C - Task 3 ----------------------------------------------------------------
 Project     : Exam PG3401 Spring 2026 - Task 3
 Author      : Candidate 102
 Revision    : 1.0
 Description : Main function for task 3 - Exam grading system
---------------------------------------------------------------------------------------*/


/*=== Standard C library: ===============================================================
=======================================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*=== Project specific include files: ===================================================
=======================================================================================*/

#include "task3_header.h"
#include "task3_util.h"


/*=== Menu enum: ========================================================================
=======================================================================================*/

enum MENUSELECTION {
    MENU_UNINITIALIZED = 0,
    MENU_ADDCANDIDATE  = 1,
    MENU_ADDFROMFILE   = 2,
    MENU_ASSESSTASK    = 3,
    MENU_PRINTALL      = 4,
    MENU_PRINTBYGRADE  = 5,
    MENU_PRINTUNGRADED = 6,
    MENU_PRINTDETAILS  = 7,
    MENU_EXITCODE      = 9
};


/*=== Forward declarations: =============================================================
=======================================================================================*/

enum MENUSELECTION ReadMenuInput(void);
void PrintGradingScreen(EXAM_LIST *pList);


/****************************************************************************************
*** MAIN FUNCTION
****************************************************************************************/

int main(void) {
    EXAM_LIST list;
    enum MENUSELECTION eMenuSelection      = MENU_UNINITIALIZED;
    char szBuffer[CANDIDATE_ID_MAX]         = { 0 };
    char szFilename[256]                    = { 0 };
    char szJustification[JUSTIFICATION_MAX] = { 0 };
    char szGrade[4]                         = { 0 };
    int iTaskIndex                          = 0;
    int iPoints                             = 0;

    /* Initialize both ends of the doubly linked list to a known state */
    list.pHead = NULL;
    list.pTail = NULL;

    do {
        PrintGradingScreen(&list);
        eMenuSelection = ReadMenuInput();

        switch(eMenuSelection) {
        case MENU_ADDCANDIDATE:
            /* Read candidate ID from user - GetStringInput prevents buffer overflow */
            printf("\n\nPlease enter Candidate ID: ");
            GetStringInput(szBuffer, sizeof(szBuffer));
            AddCandidate(&list, szBuffer);
            break;

        case MENU_ADDFROMFILE:
            /* Filename error handling is done inside AddCandidatesFromFile */
            printf("\n\nPlease enter filename: ");
            GetStringInput(szFilename, sizeof(szFilename));
            AddCandidatesFromFile(&list, szFilename);
            break;

        case MENU_ASSESSTASK:
            /* Assess a task for a candidate: prompt for ID, task number (1-6),
            points and justification, then call AssessTask. */
            printf("\n\nPlease enter Candidate ID: ");
            GetStringInput(szBuffer, sizeof(szBuffer));

            if(!CandidateExists(&list, szBuffer)) {
                printf("Error: Candidate %s not found.\n", szBuffer);
                break;
            }

            do {
                GetIntegerInput("\n\nPlease enter task number (1-6): ", &iTaskIndex);
                if(iTaskIndex < 1 || iTaskIndex > TASKS_MAX) {
                    printf("Error: Task number must be between 1 and %d.\n", TASKS_MAX);
                }
            } while(iTaskIndex < 1 || iTaskIndex > TASKS_MAX);

            iTaskIndex--; /* Convert from 1-based user input to 0-based index */

            GetIntegerInput("\n\nPlease enter points: ", &iPoints);

            printf("\n\nPlease enter justification: ");
            GetStringInput(szJustification, sizeof(szJustification));

            AssessTask(&list, szBuffer, iTaskIndex, iPoints, szJustification);
            break;

        case MENU_PRINTALL:
            PrintAllCandidates(&list);
            printf("\nPress <enter> to continue...");
            ReadMenuInput();
            break;

        case MENU_PRINTBYGRADE:
            /* Read a single grade letter and validate it before passing it on */
            printf("\n\nPlease enter grade (A/B/C/D/E/F): ");
            GetStringInput(szGrade, sizeof(szGrade));

            if(szGrade[0] == '\0' || szGrade[1] != '\0') {
                printf("Error: Please enter a single grade letter (A-F).\n");
            } else {
                PrintCandidatesByGrade(&list, szGrade[0]);
            }

            printf("\nPress <enter> to continue...");
            ReadMenuInput();
            break;

        case MENU_PRINTUNGRADED:
            PrintUngradedCandidates(&list);
            printf("\nPress <enter> to continue...");
            ReadMenuInput();
            break;

        case MENU_PRINTDETAILS:
            /* Print per-task scores and justifications for a specific candidate */
            printf("\n\nPlease enter Candidate ID: ");
            GetStringInput(szBuffer, sizeof(szBuffer));

            PrintCandidateDetails(&list, szBuffer);
            printf("\nPress <enter> to continue...");
            ReadMenuInput();
            break;

        default:
            /* Always good to have a default case even when all values are covered */
            break;

        } /* end switch (eMenuSelection) */

    } while(eMenuSelection != MENU_EXITCODE);

    /* Free all allocated memory before we leave */
    FreeAllCandidates(&list);
    memset(&list, 0, sizeof(EXAM_LIST));
    printf("\nMemory freed.\n");

    return 0;
}


/*=== PrintGradingScreen: ===============================================================
=======================================================================================*/

void PrintGradingScreen(EXAM_LIST *pList) {
    EXAM_CANDIDATE *pTemp = pList->pHead;
    int iCount = 0;

    while(pTemp != NULL) {
        iCount++;
        pTemp = pTemp->pNext;
    }

    printf("\n");
    printf("  +==============================================================+\n");
    printf("  |        EXAM GRADING SYSTEM - PG3401 Spring 2026              |\n");
    printf("  +==============================================================+\n");
    printf("  |  Candidates loaded: %-3d                                      |\n", iCount);
    printf("  +--------------------------------------------------------------+\n");
    printf("  |  1. Add candidate manually                                   |\n");
    printf("  |  2. Add candidates from file                                 |\n");
    printf("  |  3. Assess a task for a candidate                            |\n");
    printf("  |  4. Print all candidates                                     |\n");
    printf("  |  5. Print candidates by grade                                |\n");
    printf("  |  6. Print ungraded candidates                                |\n");
    printf("  |  7. Print candidate details                                  |\n");
    printf("  +--------------------------------------------------------------+\n");
    printf("  |  9. Exit                                                     |\n");
    printf("  +==============================================================+\n");
    printf("  Enter choice: ");
}


/*=== ReadMenuInput: ====================================================================
=======================================================================================*/

enum MENUSELECTION ReadMenuInput(void) {
    char szBuffer[8] = { 0 };
    int iSelection   = 0;

    GetStringInput(szBuffer, sizeof(szBuffer));
    iSelection = atoi(szBuffer);

    if(iSelection == MENU_ADDCANDIDATE)  return MENU_ADDCANDIDATE;
    if(iSelection == MENU_ADDFROMFILE)   return MENU_ADDFROMFILE;
    if(iSelection == MENU_ASSESSTASK)    return MENU_ASSESSTASK;
    if(iSelection == MENU_PRINTALL)      return MENU_PRINTALL;
    if(iSelection == MENU_PRINTBYGRADE)  return MENU_PRINTBYGRADE;
    if(iSelection == MENU_PRINTUNGRADED) return MENU_PRINTUNGRADED;
    if(iSelection == MENU_PRINTDETAILS)  return MENU_PRINTDETAILS;
    if(iSelection == MENU_EXITCODE)      return MENU_EXITCODE;

    printf("Invalid selection: %i\n", iSelection);
    return MENU_UNINITIALIZED;
}

/*END OF FILE ----------------------------------------------------------------*/

