/* TASK2_HEADER.H - Task 2 ---------------------------------------------------------

Project     : Exam PG3401 Spring 2026 - Task 2
Author      : Candidate 102
Description : Header file for task 2 - type definitions and function declarations

----------------------------------------------------------------------------------*/

#ifndef TASK2_H
#define TASK2_H

typedef int bool;
#define true 1
#define false 0

/*******************************************************************************
*** STRUCT DEFINITIONS
*******************************************************************************/

/* Aggregates all computed properties for a single integer entry */
typedef struct NUMBERS_METADATA {
    int iIndex;
    int iNumber;
    int bIsFibonacci;
    int bIsPrimeNumber;
    int bIsSquareNumber;
    int bIsCubeNumber;
    int bIsPerfectNumber;
    int bIsAbundantNumber;
    int bIsDeficientNumber;
    int bIsOddNumber;
  } NUMBERS_METADATA;

/*******************************************************************************
*** FUNCTION DECLARATIONS
*******************************************************************************/

  bool isFibonacci(int n);
  bool isPrime(int n);
  bool isSquareNumber(int n);
  bool isCubeNumber(int n);
  bool isPerfectNumber(int n);
  bool isAbundantNumber(int n);
  bool isDeficientNumber(int n);
  bool isOdd(int n);

#endif

/*END OF FILE ----------------------------------------------------------------*/
