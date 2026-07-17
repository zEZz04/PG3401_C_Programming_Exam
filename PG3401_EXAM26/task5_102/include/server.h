/* SERVER.H - Task 5 -------------------------------------------------------------------
 Project     : Exam PG3401 Spring 2026 - Task 5
 Author      : Candidate 102
 Description : Prototypes for server socket creation and client accept logic
---------------------------------------------------------------------------------------*/

#ifndef SERVER_H
#define SERVER_H


/*=== Constants: ========================================================================
=======================================================================================*/

#define MAX_CLIENTS 5


/*=== Function prototypes: ==============================================================
=======================================================================================*/

int CreateServerSocket(int iPort);
int AcceptClient(int iServerSocket);


#endif

/*END OF FILE ----------------------------------------------------------------*/

