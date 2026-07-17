/* TASK4_THREADS.C - Task 4 ------------------------------------------------------------
 Project     : Exam PG3401 Spring 2026 - Task 4
 Author      : Candidate 102
 Description : Two threads read numbers from file, find primes, and try each prime
               as a XTEA key to decrypt task4_code.bin. First thread to find the
               correct key writes the plaintext and checksum to disk.
---------------------------------------------------------------------------------------*/


/*=== Standard C library: ===============================================================
=======================================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>


/*=== Project specific include files: ===================================================
=======================================================================================*/

#include "include/task4_prime.h"


/*=== Forward declarations: =============================================================
=======================================================================================*/

void tean(unsigned int *const v, unsigned int *const w, const unsigned int *const k, int N);
unsigned int tcp_checksum(const unsigned char *data, size_t length);


/*=== Data structures: ==================================================================
=======================================================================================*/

/* Linked list node to store a found prime number */
struct PGPRIMENUMBER {
    unsigned int          uiPrime;
    struct PGPRIMENUMBER *pNext;
};

/* Passed to both threads so they share file, list and control state
   without needing any global variables */
typedef struct {
    FILE                 *fInputFile;
    struct PGPRIMENUMBER *pPrimeList;
    pthread_mutex_t       fileMutex;
    pthread_mutex_t       listMutex;
    pthread_mutex_t       doneMutex;
    int                   iDone;
    unsigned char        *pEncData;
    size_t                uiEncSize;
} PGTHREADARGS;


/****************************************************************************************
*** THREAD FUNCTION
****************************************************************************************/

void *threadFunction(void *arg) {
    PGTHREADARGS         *pArgs;
    unsigned int          auiNumbers[10];
    int                   iNumbersRead;
    int                   iIndex;
    int                   iDoneLocal;
    size_t                i;
    struct PGPRIMENUMBER *pNewNode;
    unsigned int          key[4];
    unsigned int          v[2];
    unsigned int          w[2];
    unsigned char        *pDecrypted;

    /* Cast void pointer back to our thread args struct */
    pArgs        = (PGTHREADARGS *)arg;
    iNumbersRead = 0;
    iIndex       = 0;
    iDoneLocal   = 0;
    i            = 0;
    pNewNode     = NULL;
    pDecrypted   = NULL;
    memset(auiNumbers, 0, sizeof(auiNumbers));
    memset(key,        0, sizeof(key));
    memset(v,          0, sizeof(v));
    memset(w,          0, sizeof(w));

    if(pArgs == NULL) {
        pthread_exit(NULL);
    }

    while(1) {
        iNumbersRead = 0;

        /* Check if the other thread already found the key */
        pthread_mutex_lock(&pArgs->doneMutex);
        iDoneLocal = pArgs->iDone;
        pthread_mutex_unlock(&pArgs->doneMutex);

        if(iDoneLocal) {
            break;
        }

        /* Only one thread reads from the file at a time - without this
           both threads would corrupt each other file position */
        pthread_mutex_lock(&pArgs->fileMutex);

        while(iNumbersRead < 10) {
            if(fscanf(pArgs->fInputFile, "%u", &(auiNumbers[iNumbersRead])) == 1) {
                iNumbersRead++;
            } else {
                break;
            }
        }

        pthread_mutex_unlock(&pArgs->fileMutex);

        /* No numbers read means we have reached end of file */
        if(iNumbersRead == 0) {
            break;
        }

        for(iIndex = 0; iIndex < iNumbersRead; iIndex++) {
            if(isPrime(auiNumbers[iIndex])) {

                /* Add the prime to the shared list, one thread at a time
                   to prevent nodes from overwriting each other */
                pNewNode = (struct PGPRIMENUMBER *)malloc(sizeof(struct PGPRIMENUMBER));
                if(pNewNode != NULL) {
                    pNewNode->uiPrime = auiNumbers[iIndex];

                    pthread_mutex_lock(&pArgs->listMutex);
                    pNewNode->pNext    = pArgs->pPrimeList;
                    pArgs->pPrimeList  = pNewNode;
                    pNewNode           = NULL;
                    pthread_mutex_unlock(&pArgs->listMutex);
                }

                /* Check again before attempting the decrypt, the other thread
                   may have found the key while we were adding to the list */
                pthread_mutex_lock(&pArgs->doneMutex);
                iDoneLocal = pArgs->iDone;
                pthread_mutex_unlock(&pArgs->doneMutex);

                if(iDoneLocal) {
                    break;
                }

                /* Use the prime as all four XTEA key words */
                key[0] = auiNumbers[iIndex];
                key[1] = auiNumbers[iIndex];
                key[2] = auiNumbers[iIndex];
                key[3] = auiNumbers[iIndex];

                pDecrypted = (unsigned char *)malloc(pArgs->uiEncSize);
                if(pDecrypted == NULL) {
                    continue;
                }
                memset(pDecrypted, 0, pArgs->uiEncSize);

                /* Decrypt 8 bytes at a time, negative N means decode */
                for(i = 0; i + 7 < pArgs->uiEncSize; i += 8) {
                    memcpy(&v[0], pArgs->pEncData + i,     4);
                    memcpy(&v[1], pArgs->pEncData + i + 4, 4);
                    tean(v, w, key, -32);
                    memcpy(pDecrypted + i,     &w[0], 4);
                    memcpy(pDecrypted + i + 4, &w[1], 4);
                }

                if(memcmp(pDecrypted, "BENGT", 5) == 0) {
                    FILE        *fpOut;
                    FILE        *fpHash;
                    unsigned int uiChecksum;

                    /* Signal the other thread to stop */
                    pthread_mutex_lock(&pArgs->doneMutex);
                    pArgs->iDone = 1;
                    pthread_mutex_unlock(&pArgs->doneMutex);

                    printf("\n*** KEY FOUND: %u ***\n", auiNumbers[iIndex]);

                    /* Write decrypted plaintext to disk */
                    fpOut = fopen("task4_plain.txt", "wb");
                    if(fpOut != NULL) {
                        fwrite(pDecrypted, 1, pArgs->uiEncSize, fpOut);
                        fclose(fpOut);
                    }

                    /* Calculate checksum and write to hash file */
                    uiChecksum = tcp_checksum(pDecrypted, pArgs->uiEncSize);
                    fpHash = fopen("task4_plain.hash", "wb");
                    if(fpHash != NULL) {
                        fwrite(&uiChecksum, sizeof(uiChecksum), 1, fpHash);
                        fclose(fpHash);
                    }

                    free(pDecrypted);
                    pDecrypted = NULL;
                    break;
                }

                free(pDecrypted);
                pDecrypted = NULL;
            }
        }
    }

    pthread_exit(NULL);
}


/****************************************************************************************
*** MAIN FUNCTION
****************************************************************************************/

int main(int argc, char *argv[]) {
    pthread_t             thread1;
    pthread_t             thread2;
    PGTHREADARGS          threadArgs;
    struct PGPRIMENUMBER *pPtr;
    FILE                 *fpEnc;
    size_t                uiBytesRead;
    int                   rc;

    pPtr        = NULL;
    fpEnc       = NULL;
    uiBytesRead = 0;
    rc          = 0;
    memset(&threadArgs, 0, sizeof(threadArgs));

    threadArgs.pPrimeList = NULL;
    threadArgs.fInputFile = NULL;
    threadArgs.iDone      = 0;
    threadArgs.pEncData   = NULL;
    threadArgs.uiEncSize  = 0;

    if(argc < 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    threadArgs.fInputFile = fopen(argv[1], "r");
    if(threadArgs.fInputFile == NULL) {
        perror("Error opening input file");
        return 1;
    }

    /* Load the encrypted binary into memory once, both threads read from
       this shared buffer so we only pay the disk cost one time */
    fpEnc = fopen("task4_code.bin", "rb");
    if(fpEnc == NULL) {
        perror("Could not open task4_code.bin");
        fclose(threadArgs.fInputFile);
        return 1;
    }

    fseek(fpEnc, 0, SEEK_END);
    threadArgs.uiEncSize = (size_t)ftell(fpEnc);
    rewind(fpEnc);

    threadArgs.pEncData = (unsigned char *)malloc(threadArgs.uiEncSize);
    if(threadArgs.pEncData == NULL) {
        perror("Memory allocation failed");
        fclose(fpEnc);
        fclose(threadArgs.fInputFile);
        return 1;
    }

    uiBytesRead = fread(threadArgs.pEncData, 1, threadArgs.uiEncSize, fpEnc);
    fclose(fpEnc);
    printf("Loaded task4_code.bin (%u bytes)\n", (unsigned int)uiBytesRead);

    /* Init all mutexes before any thread can touch shared data */
    pthread_mutex_init(&threadArgs.fileMutex, NULL);
    pthread_mutex_init(&threadArgs.listMutex, NULL);
    pthread_mutex_init(&threadArgs.doneMutex, NULL);

    /* Start thread 1 */
    rc = pthread_create(&thread1, NULL, threadFunction, (void *)&threadArgs);
    if(rc != 0) {
        perror("Could not create thread 1");
        fclose(threadArgs.fInputFile);
        free(threadArgs.pEncData);
        pthread_mutex_destroy(&threadArgs.fileMutex);
        pthread_mutex_destroy(&threadArgs.listMutex);
        pthread_mutex_destroy(&threadArgs.doneMutex);
        return 1;
    }

    /* Start thread 2, if this fails we join thread 1 before cleaning up */
    rc = pthread_create(&thread2, NULL, threadFunction, (void *)&threadArgs);
    if(rc != 0) {
        perror("Could not create thread 2");
        pthread_join(thread1, NULL);
        fclose(threadArgs.fInputFile);
        free(threadArgs.pEncData);
        pthread_mutex_destroy(&threadArgs.fileMutex);
        pthread_mutex_destroy(&threadArgs.listMutex);
        pthread_mutex_destroy(&threadArgs.doneMutex);
        return 1;
    }

    /* Block until both threads are done */
    if(pthread_join(thread1, NULL) != 0) {
        perror("Could not join thread 1");
    }
    if(pthread_join(thread2, NULL) != 0) {
        perror("Could not join thread 2");
    }

    fclose(threadArgs.fInputFile);

    pthread_mutex_destroy(&threadArgs.fileMutex);
    pthread_mutex_destroy(&threadArgs.listMutex);
    pthread_mutex_destroy(&threadArgs.doneMutex);

    /* Print all primes found during the search */
    printf("\nPrime numbers found:\n");
    pPtr = threadArgs.pPrimeList;
    while(pPtr != NULL) {
        printf("%u\n", pPtr->uiPrime);
        pPtr = pPtr->pNext;
    }

    /* Free the prime list */
    while(threadArgs.pPrimeList != NULL) {
        pPtr                  = threadArgs.pPrimeList;
        threadArgs.pPrimeList = threadArgs.pPrimeList->pNext;
        free(pPtr);
        pPtr = NULL;
    }

    /* Free the encrypted data buffer */
    free(threadArgs.pEncData);
    threadArgs.pEncData = NULL;

    return 0;
}

/*END OF FILE ----------------------------------------------------------------*/

