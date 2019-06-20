#include "comm.h"

void checkFile(char* fileName, int* fd);
uint32_t getBalance(int fd, char account);
int updateBalance(int fd, char account, uint32_t balance, int16_t amount);
void initializeSemaphores();
void initializeDescriptors(int* shm_fd);
int isValidAccount(char account);

int main(int argc, char* argv[]) {
   if (argc < 2) {
      printf(NO_PARAMETER);
      exit(1);
   }
   
   int shm_fd;
   initializeDescriptors(&shm_fd);

   int fd;
   checkFile(argv[1], &fd);

   initializeSemaphores();

   char account;
   uint32_t balance;
   int16_t amount;
   int message;

   while(1) {
      sem_wait(clientRequest);

      account = rptr->buff[0];

      if (isValidAccount(account) == 0) {
	 memcpy(rptr->buff, ACCOUNT_NOT_VALID, sizeof(ACCOUNT_NOT_VALID));
	 sem_post(serverResponse);
	 continue;
      }

      balance = getBalance(fd, account); 
      memcpy(rptr->buff, &balance, sizeof(uint32_t));
     
      sem_post(serverResponse);     

      sem_wait(clientRequest);

      memcpy(&amount, rptr->buff, sizeof(int16_t));
      message = updateBalance(fd, account, balance, amount);

      if (message == 0) {
         memcpy(rptr->buff, SUCCESSFULL_TRANSACTION, sizeof(SUCCESSFULL_TRANSACTION));
      } else if (message == 1) {
         memcpy(rptr->buff, ERROR_CANNOT_WITHDRAW, sizeof(ERROR_CANNOT_WITHDRAW));
      } else {
         memcpy(rptr->buff, ERROR_CANNOT_DEPOSIT, sizeof(ERROR_CANNOT_DEPOSIT));
      }
      
      sem_post(serverResponse);
   }
   
   munmap(NULL, sizeof(struct Region));
   shm_unlink(MEMORY_NAME);
   sem_unlink(SEM_NAME);
   sem_unlink(SEM_NAME_2);
   sem_unlink(SEM_NAME_3);
   close(fd);
}

void checkFile(char* fileName, int* fd) {
   *fd = open(fileName,  O_RDWR);
   if (*fd < 0) {
      *fd = open(fileName, O_CREAT | O_RDWR, MODE);
      uint32_t value = 0;
      for (int i = 0; i < NUM_ACCOUNTS; i++) {
         if (write(*fd, &value, sizeof(uint32_t)) == 0) {
	    printf(WRITE_ERROR);
	 }
      }
   }
}

uint32_t getBalance(int fd, char account) {
   lseek(fd, (account - 'A') * sizeof(uint32_t), SEEK_SET);
   uint32_t balance;
   if (read(fd, &balance, sizeof(uint32_t)) == 0) {
      printf(READ_ERROR);
   }
   return balance;
}

int updateBalance(int fd, char account, uint32_t balance, int16_t amount) {
   if (amount >= 0) {
      if ((uint32_t)balance > (uint32_t)(UINT32_MAX - amount)) return 2;
      balance += amount;
      lseek(fd, (account - 'A') * sizeof(uint32_t), SEEK_SET);
      if (write(fd, &balance, sizeof(uint32_t)) == 0) {
         printf(WRITE_ERROR);
      }
      return 0;
   } else {
      if (balance < (uint32_t)(abs(amount))) return 1;
      balance += amount;
      lseek(fd, (account - 'A') * sizeof(uint32_t), SEEK_SET);
      if (write(fd, &balance, sizeof(uint32_t)) == 0) {
         printf(WRITE_ERROR);
      }
      return 0;
   }
}

void initializeSemaphores() {
   clientRequest = sem_open(SEM_NAME_2, O_CREAT, MODE, 0);
   serverResponse = sem_open(SEM_NAME_3, O_CREAT, MODE, 0);
}

int isValidAccount(char account) {
   return account >= 'A' && account <= 'H' ? 1 : 0;
}

void initializeDescriptors(int* shm_fd) {
   *shm_fd = shm_open(MEMORY_NAME, O_CREAT | O_RDWR, MODE);
   if (*shm_fd < 0) {
      printf(ERROR_CANT_ALLOCATE_MEM);
      exit(1);
   }

   if (ftruncate(*shm_fd, SIZE) == -1) {
      printf(ERROR_NOT_ENOUGH_SPACE);
      shm_unlink(MEMORY_NAME);
      exit(1);
   }

   rptr = mmap(NULL, sizeof(struct Region), PROT_READ | PROT_WRITE, MAP_SHARED, *shm_fd, 0);
   if (rptr == MAP_FAILED) {
      printf(ERROR_MAPPING_FAILED);
      shm_unlink(MEMORY_NAME);
      exit(1);
   }
}


