/* TASK3_UTIL.H - Task 3 ---------------------------------------------------------------
 Project     : Exam PG3401 Spring 2026 - Task 3
 Author      : Candidate 102
 Description : Prototypes for input handling, validation and grading utilities
---------------------------------------------------------------------------------------*/

#ifndef TASK3_UTIL_H
#define TASK3_UTIL_H

#include "task3_header.h"


/*=== Function prototypes: ==============================================================
=======================================================================================*/

/*** Input: ****************************************************************************/

void ClearStdin(void);
void StripWhitespace(char *szText);
void GetStringInput(char *pszBuffer, int iMaxLength);
int  GetIntegerInput(char *prompt, int *piBuffer);

/*** Validation: ***********************************************************************/

int  IsValidCandidateID(char szCandidateID[]);
int  IsValidJustification(char justification[]);

/*** Grading: **************************************************************************/

char CalculateGrade(int finalScore);


#endif 

/*END OF FILE ----------------------------------------------------------------*/

