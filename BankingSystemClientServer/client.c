#include "comm.h"

void initializeSemaphores();
void initializeDescriptors(int* shm_fd);
void errorHandle(char* message);
void validateInput(int argc, char* argv[]);

int main(int argc, char* argv[]) {
   validateInput(argc, argv);

   int shm_fd;   
   initializeDescriptors(&shm_fd);

   initializeSemaphores();

   uint32_t balance;
   int16_t amount;
   char message[SIZE];

   sem_wait(mutex);

   rptr->buff[0] = argv[1][0];

   sem_post(clientRequest);

   sem_wait(serverResponse);
   
   memcpy(message, rptr->buff, SIZE);

   errorHandle(message);

   memcpy(&balance, rptr->buff, sizeof(uint32_t));
   printf(BALANCE_OUTPUT, balance);
   
   if (scanf("%hi", &amount) < 0) {
      printf(READ_ERROR);
   }
   memcpy(rptr->buff, &amount, sizeof(int16_t));
    
   sem_post(clientRequest);
   
   sem_wait(serverResponse);

   memcpy(message, rptr->buff, SIZE);
   
   errorHandle(message);

   printf("%s\n", message);
   
   sem_post(mutex);
   
   munmap(NULL, sizeof(struct Region));
}

void initializeSemaphores() {
   mutex = sem_open(SEM_NAME, O_CREAT, MODE, 1);
   clientRequest = sem_open(SEM_NAME_2, O_CREAT, MODE, 0);
   serverResponse = sem_open(SEM_NAME_3, O_CREAT, MODE, 0);
}


void initializeDescriptors(int* shm_fd) {
   *shm_fd = shm_open(MEMORY_NAME, O_CREAT | O_RDWR, MODE);
   if (*shm_fd < 0) {
      printf(ERROR_CANT_ALLOCATE_MEM);
      exit(1);
   }

   rptr = mmap(NULL, sizeof(struct Region), PROT_READ | PROT_WRITE, MAP_SHARED, *shm_fd, 0);
   if (rptr == MAP_FAILED) {
      printf(ERROR_MAPPING_FAILED);
      shm_unlink(MEMORY_NAME);
      exit(1);
   }

}

void errorHandle(char* message) {
   if (strcmp(message, ACCOUNT_NOT_VALID) == 0 || strcmp(message, ERROR_CANNOT_WITHDRAW) == 0 || strcmp(message, ERROR_CANNOT_DEPOSIT) == 0) {
      printf("%s", message);
      munmap(NULL, sizeof(struct Region));
      sem_post(mutex);
      exit(1);
   }
}

void validateInput(int argc, char* argv[]) {
   if (argc < 2) {
       printf(NO_PARAMETER);
       exit(1);
   }

   if (strlen(argv[1]) > 1) {
       printf(ACCOUNT_NOT_VALID);
       exit(1);
   }
}
