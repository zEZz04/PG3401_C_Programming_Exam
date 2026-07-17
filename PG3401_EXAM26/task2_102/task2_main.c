/* TASK2_MAIN.C - Task 2 ----------------------------------------------------------------

Project: Exam PG3401 Spring 2026 - Task 2
Author: Candidate 102
Description: Main function for task 2, reads numbers from input file, calculates number
properties, and stores results in binary format.

---------------------------------------------------------------------------------------*/


/*=== Standard C Library: ===============================================================
=======================================================================================*/

#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>

/*=== Project spesific include files: ===================================================
=======================================================================================*/

#include "task2_header.h"


/****************************************************************************************
*** MAIN FUNCTION
****************************************************************************************/


int main(int iArgc, char *iArgv[]) {

    FILE *inputFile;
    FILE *outputFile;

    NUMBERS_METADATA metadata; 

    int iNumber;
    int iIndex = 0;

    char szFilename[64] = {0};

    /* Validate number of command line arguments */
    if (iArgc != 2) {
	printf("Error: Missing input file. Usage: %s <inputfile>\n", iArgv[0]);
	return 1;
    }

    /* Ensures that the filename fits within the buffer */
    if (strlen(iArgv[1]) >= sizeof(szFilename)) {
	puts("Error: filename too long (max 63 characters)\n");
	return 1;
    }
    
    /* Copies filename into local buffer */
    strcpy(szFilename, iArgv[1]);

    /* Open input file for reading */ 
    inputFile = fopen(szFilename, "r");
    if (inputFile == NULL) { 
    	puts("Error: Could not open input file\n");
    	return 1; 
    }

    printf("Opened input file: %s\n", szFilename);

    /* Creates output file for binary writing */
    outputFile = fopen("pgexam26_output.bin", "wb");  
    if (outputFile == NULL) {
    	puts("Could not create output file\n");
	fclose(inputFile);
	return 1; 
    }

    /* Reads integerts from file until fscanf fails */ 
    while (fscanf(inputFile, "%d", &iNumber) == 1) {
        /* Store index and value */
	metadata.iIndex = iIndex;
	metadata.iNumber = iNumber;

        /* Computes number properties */
	metadata.bIsFibonacci = isFibonacci(iNumber);
	metadata.bIsPrimeNumber = isPrime(iNumber); 
	metadata.bIsSquareNumber = isSquareNumber(iNumber);
	metadata.bIsCubeNumber = isCubeNumber(iNumber); 
	metadata.bIsPerfectNumber = isPerfectNumber(iNumber); 
	metadata.bIsAbundantNumber = isAbundantNumber(iNumber); 
	metadata.bIsDeficientNumber = isDeficientNumber(iNumber);
	metadata.bIsOddNumber = isOdd(iNumber); 

        /* Writes struct to binary file */ 
	fwrite(&metadata, sizeof(NUMBERS_METADATA), 1, outputFile);

        /* Prints information about the number */
	printf("Index: %d | Number: %d\n", metadata.iIndex, metadata.iNumber);
	
	printf("Fibonacci: %d\n", metadata.bIsFibonacci);
	printf("Prime Number: %d\n", metadata.bIsPrimeNumber);
	printf("Square Number: %d\n", metadata.bIsSquareNumber);
	printf("Cube Number: %d\n", metadata.bIsCubeNumber); 
	printf("Perfect Number: %d\n", metadata.bIsPerfectNumber);
	printf("AbundantNumber: %d\n", metadata.bIsAbundantNumber); 
	printf("DeficientNumber: %d\n", metadata.bIsDeficientNumber);
	printf("Odd Number: %d\n\n", metadata.bIsOddNumber); 

        /* Incement index for the next element */
	iIndex++;

    }

	fclose(inputFile);
	fclose(outputFile);
	
	printf("Successfully created binary output file pgexam26_output.bin\n"); 
	
	return 0;

    }

/*END OF FILE ----------------------------------------------------------------*/



