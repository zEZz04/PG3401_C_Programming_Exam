/* TASK3_HEADER.H - Task 3 --------------------------------------------------------------
 Project     : Exam PG3401 Spring 2026 - Task 3
 Author      : Candidate 102
 Description : Struct definitions and function prototypes for the exam grading system
---------------------------------------------------------------------------------------*/

#ifndef TASK3_H
#define TASK3_H


/*=== Constants: ========================================================================
=======================================================================================*/

#define CANDIDATE_ID_MAX  16
#define JUSTIFICATION_MAX 256
#define TASKS_MAX         6


/*=== Structs: ==========================================================================
=======================================================================================*/

typedef struct EXAM_CANDIDATE {
    struct EXAM_CANDIDATE *pPrev;
    struct EXAM_CANDIDATE *pNext;
    char szCandidateID[CANDIDATE_ID_MAX];
    int  iTaskPoints[TASKS_MAX];
    char szTaskJustification[TASKS_MAX][JUSTIFICATION_MAX];
    int  iFinalScore;
} EXAM_CANDIDATE;

typedef struct EXAM_LIST {
    EXAM_CANDIDATE *pHead;
    EXAM_CANDIDATE *pTail;
} EXAM_LIST;


/*=== Function prototypes: ==============================================================
=======================================================================================*/

/*** List management: ******************************************************************/

void AddCandidate(EXAM_LIST *pList, char szCandidateID[]);
void AddCandidatesFromFile(EXAM_LIST *pList, char filename[]);
void FreeAllCandidates(EXAM_LIST *pList);
int  CandidateExists(EXAM_LIST *pList, char szCandidateID[]);

/*** Assessment: ***********************************************************************/

void AssessTask(EXAM_LIST *pList, char szCandidateID[], int taskIndex, int points, char justification[]);
char CalculateGrade(int finalScore);

/*** Print functions: ******************************************************************/

void PrintAllCandidates(EXAM_LIST *pList);
void PrintCandidatesByGrade(EXAM_LIST *pList, char grade);
void PrintUngradedCandidates(EXAM_LIST *pList);
void PrintCandidateDetails(EXAM_LIST *pList, char szCandidateID[]);

#endif 

/*END OF FILE ----------------------------------------------------------------*/

