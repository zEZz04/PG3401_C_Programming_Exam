/* TASK3_LIST.C - Task 3 ---------------------------------------------------------------
 Project     : Exam PG3401 Spring 2026 - Task 3
 Author      : Candidate 102
 Description : Doubly linked list management - add, free and search candidates
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


/*=== AddCandidate: =====================================================================
=======================================================================================*/

void AddCandidate(EXAM_LIST *pList, char szCandidateID[]) {
    EXAM_CANDIDATE *pNew;
    EXAM_CANDIDATE *pTemp = pList->pHead;
    int i;

    if(!IsValidCandidateID(szCandidateID)) return;

    /* Walk the list to make sure this ID is not already registered */
    while(pTemp != NULL) {
        if(strcmp(pTemp->szCandidateID, szCandidateID) == 0) {
            printf("Error: Candidate %s already exists.\n", szCandidateID);
            return;
        }
        pTemp = pTemp->pNext;
    }

    pNew = malloc(sizeof(EXAM_CANDIDATE));
    if(pNew == NULL) {
        printf("Error: Memory allocation failed in AddCandidate.\n");
        return;
    }

    strcpy(pNew->szCandidateID, szCandidateID);

    /* -1 means unassessed, distinguishes zero points from not graded yet */
    for(i = 0; i < TASKS_MAX; i++) {
        pNew->iTaskPoints[i]           = -1;
        pNew->szTaskJustification[i][0] = '\0';
    }

    pNew->iFinalScore = 0;
    pNew->pNext       = NULL;
    pNew->pPrev       = NULL;

    /* Append to tail, keeps insertion order */
    if(pList->pHead == NULL) {
        pList->pHead = pNew;
        pList->pTail = pNew;
    } else {
        pNew->pPrev          = pList->pTail;
        pList->pTail->pNext  = pNew;
        pList->pTail         = pNew;
    }
}


/*=== AddCandidatesFromFile: ============================================================
=======================================================================================*/

void AddCandidatesFromFile(EXAM_LIST *pList, char filename[]) {
    FILE *pFile;
    char szCandidateID[CANDIDATE_ID_MAX];
    int iCount = 0;

    pFile = fopen(filename, "r");
    if(pFile == NULL) {
        printf("Error: Could not open file: %s\n", filename);
        return;
    }

    while(fscanf(pFile, "%15s", szCandidateID) == 1) {
        AddCandidate(pList, szCandidateID);
        iCount++;
    }

    fclose(pFile);

    if(iCount == 0) {
        printf("Warning: File %s was empty, no candidates loaded.\n", filename);
    } else {
        printf("%d candidate(s) loaded from file.\n", iCount);
    }
}


/*=== FreeAllCandidates: ================================================================
=======================================================================================*/

void FreeAllCandidates(EXAM_LIST *pList) {
    EXAM_CANDIDATE *pTemp = pList->pHead;
    EXAM_CANDIDATE *pNext;

    /* Walk the list and free each node, all the fields are fixed arrays so no
       nested frees are needed, just the node itself */
    while(pTemp != NULL) {
        pNext = pTemp->pNext;
        free(pTemp);
        pTemp = pNext;
    }

    pList->pHead = NULL;
    pList->pTail = NULL;
}


/*=== CandidateExists: ==================================================================
=======================================================================================*/

int CandidateExists(EXAM_LIST *pList, char szCandidateID[]) {
    EXAM_CANDIDATE *pTemp = pList->pHead;

    while(pTemp != NULL) {
        if(strcmp(pTemp->szCandidateID, szCandidateID) == 0) {
            return 1;
        }
        pTemp = pTemp->pNext;
    }

    return 0;
}
