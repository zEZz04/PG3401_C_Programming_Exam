/* TASK3_PRINT.C - Task 3 --------------------------------------------------------------
 Project     : Exam PG3401 Spring 2026 - Task 3
 Author      : Candidate 102
 Description : Print functions for candidates - all, by grade, ungraded and details
---------------------------------------------------------------------------------------*/


/*=== Standard C library: ===============================================================
=======================================================================================*/

#include <stdio.h>
#include <string.h>


/*=== Project specific include files: ===================================================
=======================================================================================*/

#include "task3_header.h"


/*=== PrintAllCandidates: ===============================================================
=======================================================================================*/

void PrintAllCandidates(EXAM_LIST *pList) {
    EXAM_CANDIDATE *pTemp = pList->pHead;
    char grade;

    if(pTemp == NULL) {
        printf("No candidates in list.\n");
        return;
    }

    printf("\n=== ALL CANDIDATES ===\n");

    while(pTemp != NULL) {
        grade = CalculateGrade(pTemp->iFinalScore);
        printf("Candidate ID: %s\n", pTemp->szCandidateID);
        printf("Final Score : %d\n", pTemp->iFinalScore);
        printf("Grade       : %c\n", grade);
        printf("----------------------\n");
        pTemp = pTemp->pNext;
    }
}


/*=== PrintCandidatesByGrade: ===========================================================
=======================================================================================*/

void PrintCandidatesByGrade(EXAM_LIST *pList, char grade) {
    EXAM_CANDIDATE *pTemp = pList->pHead;
    char currentGrade;
    int iFound = 0;

    if(grade != 'A' && grade != 'B' && grade != 'C' &&
       grade != 'D' && grade != 'E' && grade != 'F') {
        printf("Error: Invalid grade '%c'. Must be A, B, C, D, E or F.\n", grade);
        return;
    }

    if(pTemp == NULL) {
        printf("No candidates in list.\n");
        return;
    }

    printf("\n=== CANDIDATES WITH GRADE %c ===\n", grade);

    while(pTemp != NULL) {
        currentGrade = CalculateGrade(pTemp->iFinalScore);
        if(currentGrade == grade) {
            printf("Candidate ID: %s\n", pTemp->szCandidateID);
            printf("Final Score : %d\n", pTemp->iFinalScore);
            printf("Grade       : %c\n", grade);
            printf("----------------------\n");
            iFound = 1;
        }
        pTemp = pTemp->pNext;
    }

    if(!iFound) {
        printf("No candidates found with grade %c.\n", grade);
    }
}


/*=== PrintUngradedCandidates: ==========================================================
=======================================================================================*/

void PrintUngradedCandidates(EXAM_LIST *pList) {
    EXAM_CANDIDATE *pTemp = pList->pHead;
    int iFound     = 0;
    int iIsUngraded = 0;
    int i;

    if(pTemp == NULL) {
        printf("No candidates in list.\n");
        return;
    }

    printf("\n=== UNGRADED CANDIDATES ===\n");

    while(pTemp != NULL) {
        iIsUngraded = 0;
        for(i = 0; i < TASKS_MAX; i++) {
            /* A task is considered ungraded if points are -1 or justification is empty */
            if(pTemp->iTaskPoints[i] < 0 || pTemp->szTaskJustification[i][0] == '\0') {
                iIsUngraded = 1;
                break;
            }
        }
        if(iIsUngraded) {
            printf("Candidate ID: %s\n", pTemp->szCandidateID);
            printf("----------------------\n");
            iFound = 1;
        }
        pTemp = pTemp->pNext;
    }

    if(!iFound) {
        printf("All candidates have been fully graded.\n");
    }
}


/*=== PrintCandidateDetails: ============================================================
=======================================================================================*/

void PrintCandidateDetails(EXAM_LIST *pList, char szCandidateID[]) {
    EXAM_CANDIDATE *pTemp = pList->pHead;
    char grade;
    int i;

    if(szCandidateID[0] == '\0') {
        printf("Error: Candidate ID cannot be empty.\n");
        return;
    }

    while(pTemp != NULL) {
        if(strcmp(pTemp->szCandidateID, szCandidateID) == 0) {
            grade = CalculateGrade(pTemp->iFinalScore);
            printf("\n=== CANDIDATE DETAILS ===\n");
            printf("Candidate ID: %s\n", pTemp->szCandidateID);
            printf("Final Score : %d\n", pTemp->iFinalScore);
            printf("Grade       : %c\n", grade);
            printf("----------------------\n");
            for(i = 0; i < TASKS_MAX; i++) {
                printf("Task %d\n", i + 1);
                printf("  Points       : %d\n", pTemp->iTaskPoints[i]);
                printf("  Justification: %s\n", pTemp->szTaskJustification[i]);
                printf("----------------------\n");
            }
            return;
        }
        pTemp = pTemp->pNext;
    }

    printf("Error: Candidate %s not found.\n", szCandidateID);
}
