/* CLIENTHANDLER.C - Task 5 ------------------------------------------------------------
 Project     : Exam PG3401 Spring 2026 - Task 5
 Author      : Candidate 102
 Description : Handles connected EWA clients using the EWP protocol
---------------------------------------------------------------------------------------*/


/*=== Standard C library: ===============================================================
=======================================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>


/*=== Socket include files: =============================================================
=======================================================================================*/

#include <sys/socket.h>


/*=== Project specific include files: ===================================================
=======================================================================================*/

#include "ewpdef.h"
#include "clienthandler.h"
#include "util.h"


/*=== Constants: ========================================================================
=======================================================================================*/

#define HEADER_SIZE sizeof(struct EWA_EXAM25_TASK5_PROTOCOL_SIZEHEADER)


/*=== Server state machine enum: ========================================================
 The server follows a strict sequence of states modelled after SMTP.
 Each state defines which command is valid next. Sending a command out
 of order results in a 503 bad sequence reply to the client.
=======================================================================================*/

enum ServerState {
    STATE_WAIT_HELO,
    STATE_WAIT_MAILFROM,
    STATE_WAIT_RCPTTO,
    STATE_WAIT_DATA_CMD,
    STATE_RECEIVING_FILE,
    STATE_WAIT_QUIT
};


/*=== SendServerAccept: =================================================================
 Sends the initial 220 greeting to a newly connected client.
 This is the first thing the server sends after a connection is accepted.
=======================================================================================*/

int SendServerAccept(int iClientSocket, char *szServerId) {
    struct EWA_EXAM25_TASK5_PROTOCOL_SERVERACCEPT sAccept;
    memset(&sAccept, 0, sizeof(sAccept));

    puts("*** Running SendServerAccept() ***");

    /* Fill the protocol header fields */
    memcpy(sAccept.stHead.acMagicNumber, "EWP",  3);
    memcpy(sAccept.stHead.acDataSize,    "0064", 4);
    sAccept.stHead.acDelimeter[0] = '|';

    /* Status code 220 means the server is ready */
    memcpy(sAccept.acStatusCode, "220", 3);
    sAccept.acHardSpace[0] = 0x20;

    MakeFormattedAcceptString(sAccept.acFormattedString, szServerId);
    sAccept.acHardZero[0] = '\0';

    if(send(iClientSocket, &sAccept, sizeof(sAccept), 0) != sizeof(sAccept)) {
        fprintf(stderr, "[ERROR] Sending server accept failed\n");
        return -1;
    }

    printf("[INFO] Sent server accept (\"%s\") to client\n", sAccept.acFormattedString);
    return 0;
}


/*=== SendServerReply: ==================================================================
 Generic reply function used by all handlers. Takes a 3 character status
 code and a message string and sends them as an EWP reply struct.
=======================================================================================*/

int SendServerReply(int iClientSocket, char *szStatusCode, char *szMessage) {
    int iResult = 0;
    struct EWA_EXAM25_TASK5_PROTOCOL_SERVERREPLY sReply;
    memset(&sReply, 0, sizeof(sReply));

    /* Fill header */
    memcpy(sReply.stHead.acMagicNumber, "EWP",  3);
    memcpy(sReply.stHead.acDataSize,    "0064", 4);
    sReply.stHead.acDelimeter[0] = '|';

    /* Fill status code and message body */
    memcpy(sReply.acStatusCode, szStatusCode, 3);
    sReply.acHardSpace[0] = 0x20;

    strncpy(sReply.acFormattedString, szMessage, 49);
    sReply.acFormattedString[49] = '\0';
    sReply.acHardZero[0] = '\0';

    iResult = send(iClientSocket, &sReply, sizeof(sReply), 0);
    if(iResult != (int)sizeof(sReply)) {
        fprintf(stderr, "[ERROR] Failed to send reply (Code: %s / Message: %s)\n",
                szStatusCode, szMessage);
        return -1;
    }

    printf("[INFO] Sent reply (\"%s %s\") to client\n", szStatusCode, szMessage);
    return 0;
}


/*=== SendBadSequenceReply: =============================================================
 Convenience wrapper that sends a 503 bad sequence reply. Called whenever
 the client sends a command that is not valid in the current state.
=======================================================================================*/

int SendBadSequenceReply(int iClientSocket) {
    return SendServerReply(iClientSocket, "503", "Bad sequence of commands");
}


/*=== ReadClientCommand: ================================================================
 Peeks at the incoming data to read the header and the first 4 bytes of
 the payload without consuming them from the socket buffer. This lets us
 identify which command is coming and pass the full message to the correct
 handler, which does the actual recv.
=======================================================================================*/

int ReadClientCommand(int iClientSocket, char *pszOutCommand, int iMaxLen,
                      struct EWA_EXAM25_TASK5_PROTOCOL_SIZEHEADER *pHeader) {
    int  iBytes             = 0;
    int  iDataSize          = 0;
    int  iPeekSize          = HEADER_SIZE + 4;
    char acPeek[12]         = { 0 };
    char acCommand[5]       = { 0 };
    struct EWA_EXAM25_TASK5_PROTOCOL_SIZEHEADER sHeader;
    memset(&sHeader, 0, sizeof(sHeader));

    /* MSG_PEEK reads without removing data from the socket buffer
       so the handler can still do a full recv on the complete message */
    iBytes = recv(iClientSocket, acPeek, iPeekSize, MSG_PEEK);
    if(iBytes != iPeekSize) {
        fprintf(stderr, "[ERROR] Could not peek enough bytes to detect command\n");
        return -1;
    }

    /* Split the peeked bytes into header and command portions */
    memcpy(&sHeader,  acPeek,              HEADER_SIZE);
    memcpy(acCommand, acPeek + HEADER_SIZE, 4);

    /* Validate the magic number to make sure this is an EWP packet */
    if(strncmp(sHeader.acMagicNumber, "EWP", 3) != 0) {
        fprintf(stderr, "[ERROR] Wrong magic number, received %.3s\n", sHeader.acMagicNumber);
        return -1;
    }

    /* Validate data size is within the allowed range */
    iDataSize = atoi(sHeader.acDataSize);
    if(iDataSize <= 0 || iDataSize > 9998) {
        fprintf(stderr, "[ERROR] Invalid data size: %s\n", sHeader.acDataSize);
        return -1;
    }

    /* Validate the delimiter separating header from payload */
    if(sHeader.acDelimeter[0] != '|') {
        fprintf(stderr, "[ERROR] Wrong delimiter, got %c\n", sHeader.acDelimeter[0]);
        return -1;
    }

    /* Pass the validated header back to the caller */
    memcpy(pHeader, &sHeader, HEADER_SIZE);

    /* Map the 4 byte command string to the output buffer */
    if(strcmp(acCommand, "HELO") == 0) {
        strncpy(pszOutCommand, "HELO", iMaxLen);
    } else if(strcmp(acCommand, "MAIL") == 0) {
        strncpy(pszOutCommand, "MAIL", iMaxLen);
    } else if(strcmp(acCommand, "RCPT") == 0) {
        strncpy(pszOutCommand, "RCPT", iMaxLen);
    } else if(strcmp(acCommand, "DATA") == 0) {
        strncpy(pszOutCommand, "DATA", iMaxLen);
    } else if(strcmp(acCommand, "QUIT") == 0) {
        strncpy(pszOutCommand, "QUIT", iMaxLen);
    } else {
        strncpy(pszOutCommand, "", iMaxLen);
        fprintf(stderr, "[ERROR] Unknown command type, data size was: %d\n", iDataSize);
        return -1;
    }

    return 0;
}


/*=== HandleClientHelo: =================================================================
 Reads the HELO message from the client, extracts the hostname and IP
 address from the formatted string, and sends back a 250 greeting.
=======================================================================================*/

int HandleClientHelo(int iClientSocket,
                     struct EWA_EXAM25_TASK5_PROTOCOL_SIZEHEADER *pHeader) {
    int   iBytes            = 0;
    char  acMessage[50]     = { 0 };
    char  szSendMessage[51] = { 0 };
    char *pszName           = NULL;
    char *pszIP             = NULL;
    struct EWA_EXAM25_TASK5_PROTOCOL_CLIENTHELO sClient;
    memset(&sClient, 0, sizeof(sClient));

    /* pHeader is part of the shared handler signature but not needed here */
    (void)pHeader;

    puts("*** Running HandleClientHelo() ***");

    /* Read the full HELO struct from the socket */
    iBytes = recv(iClientSocket, &sClient, sizeof(sClient), 0);
    if(iBytes != (int)sizeof(sClient)) {
        fprintf(stderr, "[ERROR] Reading HELO message, got %d of %ld bytes\n",
                iBytes, sizeof(sClient));
        return -1;
    }

    /* Confirm the command field actually says HELO */
    if(strncmp(sClient.acCommand, "HELO", 4) != 0) {
        fprintf(stderr, "[ERROR] Wrong command in HELO: %.4s\n", sClient.acCommand);
        return -1;
    }

    /* The formatted string contains "hostname.IP" so we split on the dot */
    memcpy(acMessage, sClient.acFormattedString, sizeof(sClient.acFormattedString));
    pszName = strtok(acMessage, ".");
    pszIP   = strtok(NULL, "");

    /* Build the greeting message to send back */
    strcpy(szSendMessage, "Hello ");
    strcat(szSendMessage, pszName);
    strcat(szSendMessage, " at ");
    strcat(szSendMessage, pszIP);
    szSendMessage[49] = '\0';

    if(SendServerReply(iClientSocket, "250", szSendMessage) < 0) {
        fprintf(stderr, "[ERROR] Failed to send HELO response\n");
        return -1;
    }

    return 0;
}


/*=== HandleMailFrom: ===================================================================
 Reads the MAIL FROM message, extracts the sender address from between
 angle brackets and sends back a 250 acknowledgement.
=======================================================================================*/

int HandleMailFrom(int iClientSocket,
                   struct EWA_EXAM25_TASK5_PROTOCOL_SIZEHEADER *pHeader) {
    int   iBytes            = 0;
    int   iLen              = 0;
    char  acMessage[44]     = { 0 };
    char  acTemp[44]        = { 0 };
    char  szSendMessage[51] = { 0 };
    char *pszStart          = NULL;
    char *pszEnd            = NULL;

    struct EWA_EXAM25_TASK5_PROTOCOL_MAILFROM sClient;
    memset(&sClient, 0, sizeof(sClient));

    /* pHeader is part of the shared handler signature but not needed here */
    (void)pHeader;

    puts("*** Running HandleMailFrom() ***");

    iBytes = recv(iClientSocket, &sClient, sizeof(sClient), 0);
    if(iBytes != (int)sizeof(sClient)) {
        fprintf(stderr, "[ERROR] Reading MAIL FROM message, got %d of %ld bytes\n",
                iBytes, sizeof(sClient));
        return -1;
    }

    if(strncmp(sClient.acCommand, "MAIL FROM:", 10) != 0) {
        fprintf(stderr, "[ERROR] Wrong command in MAIL FROM: %.10s\n", sClient.acCommand);
        return -1;
    }

    memcpy(acMessage, sClient.acFormattedString, sizeof(sClient.acFormattedString));

    /* Extract the address from between the angle brackets */
    pszStart = strchr(acMessage, '<');
    pszEnd   = strchr(acMessage, '>');

    if(pszStart && pszEnd && pszStart < pszEnd) {
        iLen = pszEnd - pszStart - 1;
        if(iLen < (int)sizeof(acMessage)) {
            strncpy(acTemp, pszStart + 1, iLen);
            acTemp[iLen] = '\0';
            memcpy(acMessage, acTemp, iLen + 1);
        } else {
            SendServerReply(iClientSocket, "500", "MAIL FROM address too long");
            return -1;
        }
    } else {
        SendServerReply(iClientSocket, "500", "Syntax error in MAIL FROM address");
        return -1;
    }

    strcpy(szSendMessage, "Got MAIL FROM: ");
    strcat(szSendMessage, acMessage);
    szSendMessage[49] = '\0';

    if(SendServerReply(iClientSocket, "250", szSendMessage) < 0) {
        fprintf(stderr, "[ERROR] Failed to send MAIL FROM response\n");
        return -1;
    }

    return 0;
}


/*=== HandleRcptTo: =====================================================================
 Reads the RCPT TO message, extracts the recipient address from between
 angle brackets and sends back a 250 acknowledgement.
=======================================================================================*/

int HandleRcptTo(int iClientSocket,
                 struct EWA_EXAM25_TASK5_PROTOCOL_SIZEHEADER *pHeader) {
    int   iBytes            = 0;
    int   iLen              = 0;
    char  acMessage[46]     = { 0 };
    char  acTemp[46]        = { 0 };
    char  szSendMessage[51] = { 0 };
    char *pszStart          = NULL;
    char *pszEnd            = NULL;
    struct EWA_EXAM25_TASK5_PROTOCOL_RCPTTO sClient;
    memset(&sClient, 0, sizeof(sClient));

    /* pHeader is part of the shared handler signature but not needed here */
    (void)pHeader;

    puts("*** Running HandleRcptTo() ***");

    iBytes = recv(iClientSocket, &sClient, sizeof(sClient), 0);
    if(iBytes != (int)sizeof(sClient)) {
        fprintf(stderr, "[ERROR] Reading RCPT TO message, got %d of %ld bytes\n",
                iBytes, sizeof(sClient));
        return -1;
    }

    if(strncmp(sClient.acCommand, "RCPT TO:", 8) != 0) {
        fprintf(stderr, "[ERROR] Wrong command in RCPT TO: %.8s\n", sClient.acCommand);
        return -1;
    }

    memcpy(acMessage, sClient.acFormattedString, sizeof(sClient.acFormattedString));

    /* Extract the address from between the angle brackets */
    pszStart = strchr(acMessage, '<');
    pszEnd   = strchr(acMessage, '>');

    if(pszStart && pszEnd && pszStart < pszEnd) {
        iLen = pszEnd - pszStart - 1;
        if(iLen < (int)sizeof(acMessage)) {
            strncpy(acTemp, pszStart + 1, iLen);
            acTemp[iLen] = '\0';
            memcpy(acMessage, acTemp, iLen + 1);
        } else {
            SendServerReply(iClientSocket, "500", "RCPT TO address too long");
            return -1;
        }
    } else {
        SendServerReply(iClientSocket, "500", "Syntax error in RCPT TO address");
        return -1;
    }

    strcpy(szSendMessage, "Got RCPT TO: ");
    strcat(szSendMessage, acMessage);
    szSendMessage[49] = '\0';

    if(SendServerReply(iClientSocket, "250", szSendMessage) < 0) {
        fprintf(stderr, "[ERROR] Failed to send RCPT TO response\n");
        return -1;
    }

    return 0;
}


/*=== HandleData: =======================================================================
 Reads the DATA command and validates the filename the client wants to use.
 If the filename passes all checks, replies with 354 and signals the main
 loop to enter the file transfer state. If invalid, replies with 550.
 Validation rules: 1 to 49 chars, no path separators, exactly one period,
 must end in .eml, only alphanumeric and the characters -_. are allowed.
=======================================================================================*/

int HandleData(int iClientSocket, char *pszAcStatusCommand, char *pszFileName,
               struct EWA_EXAM25_TASK5_PROTOCOL_SIZEHEADER *pHeader) {
    int  i                  = 0;
    int  iValidFileName     = 1;
    int  iBytes             = 0;
    int  iLen               = 0;
    int  iPeriods           = 0;
    char acMessage[50]      = { 0 };
    char szSendMessage[51]  = { 0 };
    struct EWA_EXAM25_TASK5_PROTOCOL_CLIENTDATACMD sClient;
    memset(&sClient, 0, sizeof(sClient));

    /* pHeader is part of the shared handler signature but not needed here */
    (void)pHeader;

    puts("*** Running HandleData() ***");

    iBytes = recv(iClientSocket, &sClient, sizeof(sClient), 0);
    if(iBytes != (int)sizeof(sClient)) {
        fprintf(stderr, "[ERROR] Reading DATA message, got %d of %ld bytes\n",
                iBytes, sizeof(sClient));
        return -1;
    }

    if(memcmp(sClient.acCommand, "DATA", 4) != 0) {
        fprintf(stderr, "[ERROR] Wrong command in DATA: %.4s\n", sClient.acCommand);
        return -1;
    }

    memcpy(acMessage, sClient.acFormattedString, sizeof(sClient.acFormattedString));
    iLen = strlen(acMessage);

    /* Check length is within acceptable bounds */
    if(iLen == 0 || iLen >= 50) {
        fprintf(stderr, "[WARNING] Filename length not between 1 and 49 chars: %d\n", iLen);
        iValidFileName = 0;
    }

    /* Reject path traversal attempts to protect the filesystem */
    if(strstr(acMessage, "..") || strstr(acMessage, "/") || strstr(acMessage, "\\")) {
        fprintf(stderr, "[WARNING] Filename includes path separator character\n");
        iValidFileName = 0;
    }

    /* Count periods, exactly one is required */
    for(i = 0; acMessage[i] != '\0'; i++) {
        if(acMessage[i] == '.') iPeriods++;
    }
    if(iPeriods != 1) {
        fprintf(stderr, "[WARNING] Filename has %d period(s), expected exactly 1\n", iPeriods);
        iValidFileName = 0;
    }

    /* File must end in .eml as required by the protocol */
    if(iLen < 5 || strcmp((acMessage + iLen - 4), ".eml") != 0) {
        fprintf(stderr, "[ERROR] File extension is not .eml: %s\n", acMessage);
        iValidFileName = 0;
    }

    /* Only alphanumeric and the characters -_. are allowed in the base name */
    for(i = 0; i < iLen - 4; i++) {
        if(!isalnum(acMessage[i]) && acMessage[i] != '-' &&
           acMessage[i] != '_'   && acMessage[i] != '.') {
            fprintf(stderr, "[ERROR] Invalid character in filename: '%c'\n", acMessage[i]);
            iValidFileName = 0;
        }
    }

    if(iValidFileName == 1) {
        strcpy(szSendMessage, "Valid filename: ");
        strcat(szSendMessage, acMessage);
        szSendMessage[49] = '\0';

        if(SendServerReply(iClientSocket, "354", szSendMessage) < 0) {
            fprintf(stderr, "[ERROR] Failed to send 354 DATA response\n");
            return -1;
        }

        memcpy(pszFileName, acMessage, 48);
        pszFileName[48] = '\0';
        printf("[INFO] Filename accepted: \"%s\"\n", acMessage);

        /* Set status to TRAN so the main loop knows to enter transfer state */
        memcpy(pszAcStatusCommand, "TRAN", 4);
        puts("[INFO] Setting acCommandStatus to TRAN");
    } else {
        strcpy(szSendMessage, "Invalid filename: ");
        strcat(szSendMessage, acMessage);
        szSendMessage[49] = '\0';

        if(SendServerReply(iClientSocket, "550", szSendMessage) < 0) {
            fprintf(stderr, "[ERROR] Failed to send 550 DATA response\n");
            return -1;
        }
    }

    return 0;
}


/*=== HandleTransfer: ===================================================================
 Receives file data in chunks until the end of transfer marker is found.
 The marker is CRLF CRLF . CRLF as defined by the protocol. Each chunk
 is acknowledged with a 250 reply. The complete data is written to disk
 using the filename validated in HandleData.
=======================================================================================*/

int HandleTransfer(int iClientSocket, char *pszAcStatusCommand, char *pszFileName) {
    int   iBytes                 = 0;
    int   iDataSizeComing        = 0;
    int   iTotalDataSizeReceived = 0;
    char  acPeek[HEADER_SIZE];
    char *pOldContent            = NULL;
    char *acFileContent          = NULL;
    FILE *fpOutput               = NULL;
    struct EWA_EXAM25_TASK5_PROTOCOL_SIZEHEADER sHeader;
    memset(&sHeader, 0, sizeof(sHeader));
    memset(acPeek,   0, sizeof(acPeek));

    puts("*** Running HandleTransfer() ***");

    /* Start with a 1 byte buffer, it will grow with each chunk received */
    acFileContent = (char *)malloc(1);
    if(acFileContent == NULL) {
        fprintf(stderr, "[ERROR] Initial malloc failed in HandleTransfer\n");
        return -1;
    }
    acFileContent[0] = '\0';

    while(1) {
        /* Read the chunk header to find out how many bytes are coming next */
        iBytes = recv(iClientSocket, acPeek, HEADER_SIZE, 0);
        if(iBytes != (int)HEADER_SIZE) {
            fprintf(stderr, "[ERROR] Could not read chunk header in HandleTransfer\n");
            free(acFileContent);
            return -1;
        }

        memcpy(&sHeader, acPeek, HEADER_SIZE);
        iDataSizeComing = atoi(sHeader.acDataSize);

        if(iDataSizeComing < 4) {
            fprintf(stderr, "[ERROR] Incoming chunk too small (%d bytes)\n", iDataSizeComing);
            free(acFileContent);
            return -1;
        }
        if(iDataSizeComing > 9998) {
            fprintf(stderr, "[ERROR] Incoming chunk too large (%d bytes)\n", iDataSizeComing);
            free(acFileContent);
            return -1;
        }

        printf("[INFO] Awaiting %d bytes of file data\n", iDataSizeComing);

        /* Save old pointer before realloc so we can free it if realloc fails */
        pOldContent   = acFileContent;
        acFileContent = (char *)realloc(acFileContent,
                                        iDataSizeComing + iTotalDataSizeReceived + 1);
        if(acFileContent == NULL) {
            fprintf(stderr, "[ERROR] realloc failed in HandleTransfer\n");
            free(pOldContent);
            return -1;
        }

        /* Append the new chunk directly after the previously received data */
        iBytes = recv(iClientSocket,
                      acFileContent + iTotalDataSizeReceived,
                      iDataSizeComing, 0);
        if(iBytes != iDataSizeComing) {
            fprintf(stderr, "[ERROR] Chunk read mismatch, got %d of %d bytes\n",
                    iBytes, iDataSizeComing);
            free(acFileContent);
            return -1;
        }

        acFileContent[iTotalDataSizeReceived + iBytes] = '\0';
        iTotalDataSizeReceived += iBytes;

        printf("[INFO] Received %d bytes, running total: %d\n",
               iBytes, iTotalDataSizeReceived);

        if(SendServerReply(iClientSocket, "250", "Got the data") < 0) {
            fprintf(stderr, "[ERROR] Failed to send 250 chunk acknowledgement\n");
            free(acFileContent);
            return -1;
        }

        /* Check for the end of transfer marker after each chunk */
        if(strstr(acFileContent, "\r\n\r\n.\r\n")) {
            puts("[INFO] Found end of transfer marker, closing transfer loop");
            break;
        }
    }

    /* Write the complete received data to disk */
    fpOutput = fopen(pszFileName, "w");
    if(fpOutput == NULL) {
        fprintf(stderr, "[ERROR] Could not open \"%s\" for writing\n", pszFileName);
        free(acFileContent);
        return -1;
    }

    if(fwrite(acFileContent, iTotalDataSizeReceived, 1, fpOutput) != 1) {
        fprintf(stderr, "[ERROR] Writing file content to \"%s\" failed\n", pszFileName);
    } else {
        printf("[INFO] File content written to \"%s\" successfully\n", pszFileName);
    }

    fclose(fpOutput);

    /* Clear the status so the main loop knows the transfer is complete */
    pszAcStatusCommand[0] = '\0';
    free(acFileContent);
    return 0;
}


/*=== HandleQuit: =======================================================================
 Reads the QUIT command, sends a 221 closing reply and returns. The main
 loop in HandleClient sets piRunning to 0 after this returns, which shuts
 down the server cleanly.
=======================================================================================*/

int HandleQuit(int iClientSocket,
               struct EWA_EXAM25_TASK5_PROTOCOL_SIZEHEADER *pHeader) {
    int  iBytes             = 0;
    char acBuffer[128]      = { 0 };
    char szSendMessage[51]  = { 0 };
    struct EWA_EXAM25_TASK5_PROTOCOL_CLOSECOMMAND sClient;
    memset(&sClient, 0, sizeof(sClient));

    /* pHeader is part of the shared handler signature but not needed here */
    (void)pHeader;

    puts("*** Running HandleQuit() ***");

    iBytes = recv(iClientSocket, acBuffer, sizeof(sClient), 0);
    if(iBytes != (int)sizeof(sClient)) {
        fprintf(stderr, "[ERROR] Reading QUIT message, got %d of %ld bytes\n",
                iBytes, sizeof(sClient));
        return -1;
    }

    memcpy(&sClient, acBuffer, iBytes);

    if(strncmp(sClient.acCommand, "QUIT", 4) != 0) {
        fprintf(stderr, "[ERROR] Wrong command in QUIT: %.4s\n", sClient.acCommand);
        return -1;
    }

    strcpy(szSendMessage, "Goodbye! Shutting down the server now.");
    szSendMessage[49] = '\0';

    if(SendServerReply(iClientSocket, "221", szSendMessage) < 0) {
        fprintf(stderr, "[ERROR] Failed to send QUIT response\n");
        return -1;
    }

    return 0;
}


/*=== HandleClient: =====================================================================
 Main session loop for a connected client. Sends the initial server accept,
 then reads commands and dispatches them to the correct handler based on the
 current state machine state. Runs until the client sends QUIT or an error
 occurs that cannot be recovered from.
=======================================================================================*/

int HandleClient(int iClientSocket, char *szId, int *piRunning) {
    int              iResult             = 0;
    char             szFileName[50]      = { 0 };
    char             acCommandStatus[12] = { 0 };
    enum ServerState eState              = STATE_WAIT_HELO;
    struct EWA_EXAM25_TASK5_PROTOCOL_SIZEHEADER sHeader;
    memset(&sHeader, 0, sizeof(sHeader));

    /* Send the 220 greeting as soon as the client connects */
    if(SendServerAccept(iClientSocket, szId) < 0) return -1;

    while(*piRunning) {

        /* During file transfer the status is already set to TRAN by HandleData
           so we skip reading a new command and go straight to the transfer handler */
        if(strcmp(acCommandStatus, "TRAN") != 0) {
            iResult = ReadClientCommand(iClientSocket, acCommandStatus,
                                        sizeof(acCommandStatus), &sHeader);
            if(iResult < 0) {
                fprintf(stderr, "[ERROR] Could not read command from client\n");
                return -1;
            }
        }

        switch(eState) {
        case STATE_WAIT_HELO:
            if(strcmp(acCommandStatus, "HELO") == 0) {
                if(HandleClientHelo(iClientSocket, &sHeader) < 0) return -1;
                memset(&sHeader, 0, sizeof(sHeader));
                eState = STATE_WAIT_MAILFROM;
            } else {
                SendBadSequenceReply(iClientSocket);
            }
            break;

        case STATE_WAIT_MAILFROM:
            if(strcmp(acCommandStatus, "MAIL") == 0) {
                if(HandleMailFrom(iClientSocket, &sHeader) < 0) return -1;
                memset(&sHeader, 0, sizeof(sHeader));
                eState = STATE_WAIT_RCPTTO;
            } else {
                SendBadSequenceReply(iClientSocket);
            }
            break;

        case STATE_WAIT_RCPTTO:
            if(strcmp(acCommandStatus, "RCPT") == 0) {
                if(HandleRcptTo(iClientSocket, &sHeader) < 0) return -1;
                memset(&sHeader, 0, sizeof(sHeader));
                eState = STATE_WAIT_DATA_CMD;
            } else {
                SendBadSequenceReply(iClientSocket);
            }
            break;

        case STATE_WAIT_DATA_CMD:
            if(strcmp(acCommandStatus, "DATA") == 0) {
                if(HandleData(iClientSocket, acCommandStatus,
                              szFileName, &sHeader) < 0) return -1;
                memset(&sHeader, 0, sizeof(sHeader));
                eState = STATE_RECEIVING_FILE;
            } else {
                SendBadSequenceReply(iClientSocket);
            }
            break;

        case STATE_RECEIVING_FILE:
            if(strcmp(acCommandStatus, "TRAN") == 0) {
                if(HandleTransfer(iClientSocket, acCommandStatus,
                                  szFileName) < 0) return -1;
                memset(&sHeader, 0, sizeof(sHeader));
                eState = STATE_WAIT_QUIT;
            } else {
                SendBadSequenceReply(iClientSocket);
            }
            break;

        case STATE_WAIT_QUIT:
            if(strcmp(acCommandStatus, "QUIT") == 0) {
                HandleQuit(iClientSocket, &sHeader);
                *piRunning = 0;
                return 0;
            } else if(strcmp(acCommandStatus, "DATA") == 0) {
                /* The task description allows the client to send another file
                   instead of QUIT, so we loop back to the transfer state */
                if(HandleData(iClientSocket, acCommandStatus,
                              szFileName, &sHeader) < 0) return -1;
                memset(&sHeader, 0, sizeof(sHeader));
                eState = STATE_RECEIVING_FILE;
            } else {
                SendBadSequenceReply(iClientSocket);
            }
            break;

        } 

    } 

    return 0;
}

/*END OF FILE ----------------------------------------------------------------*/
