/* TASK3_ASSESSMENT.C - Task 3 ---------------------------------------------------------
 Project     : Exam PG3401 Spring 2026 - Task 3
 Author      : Candidate 102
 Description : Task assessment - assigns points and justification per task per candidate
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


/*=== Constants: ========================================================================
=======================================================================================*/

/* Maximum achievable points per task, index matches task index 0-5 */
const int iMaxPoints[TASKS_MAX] = { 5, 15, 20, 20, 20, 20 };


/*=== AssessTask: =======================================================================
=======================================================================================*/

void AssessTask(EXAM_LIST *pList, char szCandidateID[], int taskIndex, int points, char justification[]) {
    EXAM_CANDIDATE *pTemp = pList->pHead;
    int i;

    if(taskIndex < 0 || taskIndex >= TASKS_MAX) {
        printf("Error: Invalid task index %d. Must be between 0 and %d.\n", taskIndex, TASKS_MAX - 1);
        return;
    }

    if(points < 0 || points > iMaxPoints[taskIndex]) {
        printf("Error: Points must be between 0 and %d for task %d.\n", iMaxPoints[taskIndex], taskIndex + 1);
        return;
    }

    if(!IsValidJustification(justification)) return;

    while(pTemp != NULL) {
        if(strcmp(pTemp->szCandidateID, szCandidateID) == 0) {
            pTemp->iTaskPoints[taskIndex] = points;
            strcpy(pTemp->szTaskJustification[taskIndex], justification);

            /* Recalculate final score, only count tasks that have been assessed */
            pTemp->iFinalScore = 0;
            for(i = 0; i < TASKS_MAX; i++) {
                if(pTemp->iTaskPoints[i] >= 0) {
                    pTemp->iFinalScore += pTemp->iTaskPoints[i];
                }
            }

            printf("Assessment updated for candidate %s.\n", szCandidateID);
            return;
        }
        pTemp = pTemp->pNext;
    }

    printf("Error: Candidate %s not found.\n", szCandidateID);
}

/*END OF FILE ----------------------------------------------------------------*/

