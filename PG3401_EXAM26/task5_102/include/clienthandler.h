/* CLIENTHANDLER.H - Task 5 ------------------------------------------------------------
 Project     : Exam PG3401 Spring 2026 - Task 5
 Author      : Candidate 102
 Description : Prototypes for all EWA client handling and protocol logic
---------------------------------------------------------------------------------------*/

#ifndef CLIENTHANDLER_H
#define CLIENTHANDLER_H

#include "ewpdef.h"


/*=== Function prototypes: ==============================================================
=======================================================================================*/

/*** Server replies: *******************************************************************/

int SendServerAccept(int iClientSocket, char *szServerId);
int SendServerReply(int iClientSocket, char *szStatusCode, char *szMessage);
int SendBadSequenceReply(int iClientSocket);

/*** Command reading: ******************************************************************/

int ReadClientCommand(int iClientSocket, char *pszOutCommand, int iMaxLen,
                      struct EWA_EXAM25_TASK5_PROTOCOL_SIZEHEADER *pHeader);

/*** Protocol handlers: ****************************************************************/

int HandleClientHelo(int iClientSocket,
                     struct EWA_EXAM25_TASK5_PROTOCOL_SIZEHEADER *pHeader);
int HandleMailFrom(int iClientSocket,
                   struct EWA_EXAM25_TASK5_PROTOCOL_SIZEHEADER *pHeader);
int HandleRcptTo(int iClientSocket,
                 struct EWA_EXAM25_TASK5_PROTOCOL_SIZEHEADER *pHeader);
int HandleData(int iClientSocket, char *pszAcStatusCommand, char *pszFileName,
               struct EWA_EXAM25_TASK5_PROTOCOL_SIZEHEADER *pHeader);
int HandleTransfer(int iClientSocket, char *pszAcStatusCommand, char *pszFileName);
int HandleQuit(int iClientSocket,
               struct EWA_EXAM25_TASK5_PROTOCOL_SIZEHEADER *pHeader);

/*** Session loop: *********************************************************************/

int HandleClient(int iClientSocket, char *szId, int *piRunning);


#endif 

/*END OF FILE ----------------------------------------------------------------*/

