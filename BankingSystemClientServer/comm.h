#ifndef COMM_H_
#define COMM_H_

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <pthread.h>
#include <limits.h>
#include <semaphore.h>

#define MEMORY_NAME "/sharedMemory"
#define SEM_NAME "/mySemaphore"
#define SEM_NAME_2 "/mySemaphore2"
#define SEM_NAME_3 "/mySemaphore3"

#define ERROR_CANT_ALLOCATE_MEM "Cannot allocate shared memory!\n"
#define ERROR_NOT_ENOUGH_SPACE "Cannot allocate such amount of space!\n"
#define ERROR_MAPPING_FAILED "Mapping failed!\n"

#define SUCCESSFULL_TRANSACTION "Transaction is successfull!"
#define ERROR_CANNOT_WITHDRAW "Cannot withdraw that amount of money!\n"
#define ERROR_CANNOT_DEPOSIT "Cannot deposit that amount of money!\n"
#define NO_PARAMETER "No parameter given!\n"
#define WRITE_ERROR "Writing error!\n"
#define READ_ERROR "Reading error!\n"

#define SIZE 100
#define MODE 0775
#define NUM_ACCOUNTS 8
#define ACCOUNT_NOT_VALID "Account is not valid!\n"
#define BALANCE_OUTPUT "%u\n"

struct Region {
   int len;
   char buff[SIZE];
};

struct Region *rptr;

sem_t* mutex;
sem_t* clientRequest;
sem_t* serverResponse;

#endif // COMM_H

