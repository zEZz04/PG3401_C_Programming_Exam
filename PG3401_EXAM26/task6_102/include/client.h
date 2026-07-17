/* CLIENT.H - Task 6 -------------------------------------------------------------------
 Project     : Exam PG3401 Spring 2026 - Task 6
 Author      : Candidate 102
 Description : Interface for the EWA TCP over TCP network client
---------------------------------------------------------------------------------------*/

#ifndef CLIENT_H
#define CLIENT_H


/*=== Return codes: =====================================================================
=======================================================================================*/

#define CLIENT_OK     0
#define CLIENT_ERROR -1


/*=== Function prototypes: ==============================================================
=======================================================================================*/

int run_client(const char *server_ip, int port, const char *output_file);


#endif 

/*END OF FILE ----------------------------------------------------------------*/

