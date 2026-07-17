/* UTIL.H - Task 5 ---------------------------------------------------------------------
 Project     : Exam PG3401 Spring 2026 - Task 5
 Author      : Candidate 102
 Description : Prototypes for utility and helper functions
---------------------------------------------------------------------------------------*/

#ifndef UTIL_H
#define UTIL_H


/*=== Constants: ========================================================================
=======================================================================================*/

#define MAX_ID_LEN 40
#define ID_SIZE    40


/*=== Function prototypes: ==============================================================
=======================================================================================*/

int IsValidInteger(char *str);
int CheckCliInput(int argc, char *argv[], int *piPort, char *szId, int iIdSize);
int MakeFormattedAcceptString(char *buffer, char *szServerId);


#endif 

/*END OF FILE ----------------------------------------------------------------*/

