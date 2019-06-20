#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#define TEXT_TYPE 0x00
#define NUMERIC_TYPE 0x01
#define BYTE_TYPE 0x02 

#define SEG_NUM 64
#define INVALID_VALUE -1
#define VALUE_SIZE 16
#define OFFSET 8
#define INT_SIZE 4
#define SEVENTH 7
#define SIXTH 6
#define FIFTH 5
#define BUFF_SIZE 100

#define MODE 00775

#define HELP_FILE_NAME "help.txt"
#define TABLE_FILE_NAME "table.txt"

#define ERROR_CANNOT_FIND_SEGMENT "Cannot find segment with given parameter!\n"
#define ERROR_FILE_DOES_NOT_EXIST "%s does not exist!\n"
#define ERROR_WRONG_CHANGE "Wrong change of bit!\n"
#define ERROR_WRONG_EXTENSION "Wrong extension!\n"
#define ERROR_INVALID_COMMAND "Invalid command!\n"
#define ERROR_PARAMETER_NOT_ACTIVATED "Parameter not activated!\n"
#define ERROR_WRONG_PARAMETER "Wrong parameter!\n"
#define ERROR_INVALID_SEQUENCE "Invalid sequence!\n"
#define ERROR_INVALID_VALUE "Value %s is invalid!\n"

#define DEVICE_NAME "device_name\n"
#define ROM_REVISION "rom_revision\n"
#define SERIAL_NUMBER "serial_number\n"
#define BD_ADDR_PART_0 "bd_addr_part0\n"
#define BD_ADDR_PART_1 "bd_addr_part1\n"
#define BD_PASS_PART_0 "bd_pass_part0\n"
#define SERIAL_BAUDRATE "serial_baudrate\n"
#define AUDIO_BITRATE "audio_bitrate\n"
#define SLEEP_PERIOD "sleep_period\n"
#define SERIAL_PARITY "serial_parity\n"
#define SERIAL_DATA_BIT "serial_data_bit\n"
#define SERIAL_STOP_BIT "serial_stop_bit\n"
#define BD_PASS_PART_1 "bd_pass_part1\n"
#define ROM_CHECKSUM_PART_0 "rom_checksum_part0\n"
#define ROM_CHECKSUM_PART_1 "rom_checksum_part1\n"

const int SERIAL_BAUDATE_VALUES[SIXTH] = { 1200, 2400, 4800, 9600, 19200, 115200 };
const int AUDIO_BITRATE_VALUES[SEVENTH] = { 32, 96, 128, 160, 192, 256, 320 };
const int SLEEP_PERIOD_VALUES[FIFTH] = { 100, 500, 1000, 5000, 10000 };
const char SERIAL_PARITY_VALUES[INT_SIZE] = { 'N', 'E', '0' };

int getNumber(char* str);
void copyString(char* source, char* dest, int size);
int areSame(char* first, char* second, int length, int correctLength);
void getParameterValues(char* param, int* type, int* num, int* position);
void checkForBitChange(int flag, int fdWrite, int segNum, char bit, int position);  
int checkForBitActivation(int fd, int position, int flag, char bit);
void initialiseData(int* fd, char* fileName, char* parameter, int* type, int* position, int* num); 
int isValidSequence(int argc, char* argv[]);
int matchRegex(int type, char* value); 
int isValidValue(int type, int num, int position, char* value);
void validateValue(int fd, int type, int num, int position, char* value);
int isValidBit(char* bit);

void changeParameterValue(char* fileName, char* parameter, char* newVal, int flag) {
   int cnt, type, position, num, fd;
   char buff[SEG_NUM];
   initialiseData(&fd, fileName, parameter, &type, &position, &num);

   int textNum = INVALID_VALUE, numericNum = INVALID_VALUE, byteNum = INVALID_VALUE, segNum = 0;
   int number, fdWrite, digit;
   char value[VALUE_SIZE];

   while (cnt = read(fd, buff, SEG_NUM)) {
      if (buff[0] == (char)TEXT_TYPE) textNum++;
      else if (buff[0] == (char)NUMERIC_TYPE) numericNum++;
      else byteNum++;

      if (buff[0] == (char)TEXT_TYPE && type == 0 && textNum == num) {    
         fdWrite = open(fileName, O_WRONLY);
  	 checkForBitChange(flag, fdWrite, segNum, buff[1], position);
         validateValue(fd, type, num, position, newVal);        

	 for (int i = 0; i < strlen(newVal); i++) value[i] = newVal[i];
	 for (int i = strlen(newVal); i < VALUE_SIZE; i++) value[i] = (char)TEXT_TYPE;
         lseek(fdWrite, segNum * SEG_NUM + OFFSET + position * VALUE_SIZE, SEEK_SET);
	 write(fdWrite, value, VALUE_SIZE);
	 close(fdWrite);
	 close(fd);
	 return;
      }
      else if (buff[0] == (char)NUMERIC_TYPE && type == 1 && numericNum == num) {
         fdWrite = open(fileName, O_WRONLY);
         checkForBitChange(flag, fdWrite, segNum, buff[1], position);
	 validateValue(fd, type, num, position, newVal);

         lseek(fdWrite, segNum * SEG_NUM + OFFSET + position * INT_SIZE, SEEK_SET);
	 number = getNumber(newVal);
         write(fdWrite, &number, INT_SIZE);
	 close(fdWrite);
	 close(fd);
	 return;
      }
      else if (buff[0] == (char)BYTE_TYPE && type == 2 && byteNum == num) {
	 fdWrite = open(fileName, O_WRONLY);
	 checkForBitChange(flag, fdWrite, segNum, buff[1], position);
         validateValue(fd, type, num, position, newVal);
         lseek(fdWrite, segNum * SEG_NUM + OFFSET + position, SEEK_SET);
	 if (newVal[0] >= '0' && newVal[0] <= '9') {
            digit = (int)(newVal[0] - '0');
	    write(fdWrite, &digit, 1);
	 }
	 else {
	    write(fdWrite, newVal, 1);	 
	 }
	 close(fdWrite);
	 close(fd);
  	 return;
      }
      segNum++;
   }
   printf(ERROR_CANNOT_FIND_SEGMENT);
   close(fd);
}


void showValue(char* fileName, char* parameter, int flag) {
   int cnt, type, position, num, fd;
   char buff[SEG_NUM];
   initialiseData(&fd, fileName, parameter, &type, &position, &num);

   int textNum = INVALID_VALUE, numericNum = INVALID_VALUE, byteNum = INVALID_VALUE, segNum = 0;
   int fdRead, number;
   char symb[2], value[VALUE_SIZE + 1], digit;

   while (cnt = read(fd, buff, SEG_NUM)) {
      if (buff[0] == (char)TEXT_TYPE) textNum++;
      else if (buff[0] == (char)NUMERIC_TYPE) numericNum++;
      else byteNum++;

      if (buff[0] == (char)TEXT_TYPE && type == 0 && textNum == num) {
	 if (checkForBitActivation(fd, position, flag, buff[1]) == 1) {
            close(fd);
	    return;
	 }

	 for (int i = 0; i < VALUE_SIZE; i++) value[i] = buff[OFFSET + position * VALUE_SIZE + i];
	 value[VALUE_SIZE] = '\n';
	 write(1, value, VALUE_SIZE + 1);
	 close(fd);
         return;
      }
      else if (buff[0] == (char)NUMERIC_TYPE && type == 1 && numericNum == num) {
	 if (checkForBitActivation(fd, position, flag, buff[1]) == 1) {
	    close(fd);
            return;
         }

	 fdRead = open(fileName, O_RDONLY);
	 lseek(fdRead, segNum * SEG_NUM + OFFSET + position * INT_SIZE, SEEK_SET);
	 read(fdRead, &number, INT_SIZE);
	 printf("%d\n", number);
	 close(fdRead);
	 close(fd);
         return;
      } 
      else if (buff[0] == (char)BYTE_TYPE && type == 2 && byteNum == num) {
         if (checkForBitActivation(fd, position, flag, buff[1]) == 1) {
	    close(fd);
            return;
         }

         fdRead = open(fileName, O_RDONLY);
         lseek(fdRead, segNum * SEG_NUM + OFFSET + position, SEEK_SET);
         read(fdRead, symb, 1);
         if (symb[0] > '9') {
	    symb[1] = '\n';
	    write(1, symb, 2);
	 }
	 else {
	    lseek(fdRead, segNum * SEG_NUM + OFFSET + position, SEEK_SET);
 	    read(fdRead, &digit, 1);
	    printf("%d\n", (int)digit);
	 }	 
	 close(fdRead); 
	 close(fd);
 	 return;
      }
      segNum++;
   }
   printf(ERROR_CANNOT_FIND_SEGMENT);
   close(fd);   
}

void showParameters(char* fileName, int flag) {
   int cnt;
   char buff[SEG_NUM];
   int fd = open(fileName, O_RDONLY);

   if (fd < 0) {
     printf(ERROR_FILE_DOES_NOT_EXIST, fileName);
     return;
   } 

   int textNum = INVALID_VALUE, numericNum = INVALID_VALUE, byteNum = INVALID_VALUE;
   while (cnt = read(fd, buff, SEG_NUM)) {
	if (buff[0] == (char)TEXT_TYPE) textNum++;
        else if (buff[0] == (char)NUMERIC_TYPE) numericNum++;
        else byteNum++;

	if (buff[0] == (char)TEXT_TYPE && textNum == 0) {
           if ((flag == 1 && (buff[1] & (1 << SEVENTH)) >> SEVENTH == 1) || flag == 0) printf(DEVICE_NAME);
	   if ((flag == 1 && (buff[1] & (1 << SIXTH)) >> SIXTH == 1) || flag == 0) printf(ROM_REVISION);
	   if ((flag == 1 && (buff[1] & (1 << FIFTH)) >> FIFTH == 1) || flag == 0) printf(SERIAL_NUMBER);
	}
	if (buff[0] == (char)TEXT_TYPE && textNum == 1) {
	   if ((flag == 1 && (buff[1] & (1 << SEVENTH)) >> SEVENTH == 1) || flag == 0) printf(BD_ADDR_PART_0);
           if ((flag == 1 && (buff[1] & (1 << SIXTH)) >> SIXTH == 1) || flag == 0) printf(BD_ADDR_PART_1);
           if ((flag == 1 && (buff[1] & (1 << FIFTH)) >> FIFTH == 1) || flag == 0) printf(BD_PASS_PART_0);
	}
	if (buff[0] == (char)NUMERIC_TYPE && numericNum == 0) {
	   if ((flag == 1 && (buff[1] & (1 << SEVENTH)) >> SEVENTH == 1) || flag == 0) printf(SERIAL_BAUDRATE);
           if ((flag == 1 && (buff[1] & (1 << SIXTH)) >> SIXTH == 1) || flag == 0) printf(AUDIO_BITRATE);
           if ((flag == 1 && (buff[1] & (1 << FIFTH)) >> FIFTH == 1) || flag == 0) printf(SLEEP_PERIOD);
        }
	if (buff[0] == (char)BYTE_TYPE && byteNum == 0) {
	   if ((flag == 1 && (buff[1] & (1 << SEVENTH)) >> SEVENTH == 1) || flag == 0) printf(SERIAL_PARITY);
           if ((flag == 1 && (buff[1] & (1 << SIXTH)) >> SIXTH == 1) || flag == 0) printf(SERIAL_DATA_BIT);
           if ((flag == 1 && (buff[1] & (1 << FIFTH)) >> FIFTH == 1) || flag == 0) printf(SERIAL_STOP_BIT);
	}
	if (buff[0] == (char)TEXT_TYPE && textNum == 2) {
	   if ((flag == 1 && (buff[1] & (1 << SEVENTH)) >> SEVENTH == 1) || flag == 0) printf(BD_PASS_PART_1);
           if ((flag == 1 && (buff[1] & (1 << SIXTH)) >> SIXTH == 1) || flag == 0) printf(ROM_CHECKSUM_PART_0);
           if ((flag == 1 && (buff[1] & (1 << FIFTH)) >> FIFTH == 1) || flag == 0) printf(ROM_CHECKSUM_PART_1); 
	}
   }
   close(fd);
}

void showListedParameters(int argc, char* argv[], int flag) {
   for (int i = 3; i < argc; i++) {
      showValue(argv[1], argv[i], flag);
   }
}

void changeBit(char* fileName, char* parameter, char* change) {
   int cnt, type, position, num, fd;
   char buff[SEG_NUM];
   if (isValidBit(change) == 0) {
       printf(ERROR_WRONG_CHANGE);
       exit(0);
   }
   initialiseData(&fd, fileName, parameter, &type, &position, &num);
      
   int textNum = INVALID_VALUE, numericNum = INVALID_VALUE, byteNum = INVALID_VALUE, segNum = 0;
   while (cnt = read(fd, buff, SEG_NUM)) {
      if (buff[0] == (char)TEXT_TYPE) textNum++;
      else if (buff[0] == (char)NUMERIC_TYPE) numericNum++;
      else byteNum++;

      if ((buff[0] == (char)TEXT_TYPE && type == 0 && textNum == num) ||
	  (buff[0] == (char)NUMERIC_TYPE && type == 1 && numericNum == num) ||
	  (buff[0] == (char)BYTE_TYPE && type == 2 && byteNum == num)) {
         int fdWrite = open(fileName, O_WRONLY);
         char param[1];
         if (strcmp(change, "1") == 0) param[0] = (char)(buff[1] | (1 << (SEVENTH- position)));
	 else param[0] = (char)(buff[1] & ~(1 << (SEVENTH- position)));

         lseek(fdWrite, segNum * SEG_NUM + 1 , SEEK_SET);
         write(fdWrite, param, 1);
         close(fdWrite);
         close(fd);
	 return;
      }
      segNum++;
   }
   printf(ERROR_CANNOT_FIND_SEGMENT);
   close(fd);
}

void createConfigFile(int argc, char* argv[]) {
   const char* dot = strrchr(argv[1], '.');
   if (!dot || strcmp(dot + 1, "bin") != 0) {
      printf(ERROR_WRONG_EXTENSION);
      return;
   }

   if (isValidSequence(argc, argv) == 0) return;

   int fd = open(argv[1], O_CREAT | O_WRONLY | O_TRUNC, MODE);
   char buff[SEG_NUM];
    
   for (int i = INT_SIZE; i < argc; i++) { 
      if (strcmp(argv[i], "t") == 0) {
	  for (int i = 0; i < SEG_NUM; i++) buff[i] = (char)TEXT_TYPE;
	  write(fd, buff, SEG_NUM);
      } else if (strcmp(argv[i], "n") == 0) {
	  for (int i = 1; i < SEG_NUM; i++) buff[i] = (char)TEXT_TYPE;
	  buff[0] = (char)NUMERIC_TYPE;
	  write(fd, buff, SEG_NUM);
      } else if (strcmp(argv[i], "b") == 0) {
	  for (int i = 1; i < SEG_NUM; i++) buff[i] = (char)TEXT_TYPE;
	  buff[0] = (char)BYTE_TYPE;
	  write(fd, buff, SEG_NUM);
      }
   }
   close(fd);
}

void showHelp() {
   int fd = open(HELP_FILE_NAME, O_RDONLY);
   char buff[BUFF_SIZE];
   int cnt;
   while (cnt = read(fd, buff, BUFF_SIZE)) {
      write(1, buff, cnt);
   }
}

int main(int argc, char* argv[]) {
   if (argc <= 2) showHelp();
   else if (strcmp(argv[2], "-s") == 0) changeParameterValue(argv[1], argv[3], argv[INT_SIZE], 1); 
   else if (strcmp(argv[2], "-S") == 0)	changeParameterValue(argv[1], argv[3], argv[INT_SIZE], 0);
   else if (strcmp(argv[2], "-g") == 0) showValue(argv[1], argv[3], 1);   
   else if (strcmp(argv[2], "-G") == 0)	showValue(argv[1], argv[3], 0);
   else if (strcmp(argv[2], "-l") == 0 && argc == 3) showParameters(argv[1], 1);
   else if (strcmp(argv[2], "-L") == 0 && argc == 3) showParameters(argv[1], 0);
   else if (strcmp(argv[2], "-l") == 0 && argc > 3) showListedParameters(argc, argv, 1);
   else if (strcmp(argv[2], "-L") == 0 && argc > 3) showListedParameters(argc, argv, 0);
   else if (strcmp(argv[2], "-b") == 0) changeBit(argv[1], argv[3], argv[INT_SIZE]);
   else if (strcmp(argv[2], "-c") == 0) createConfigFile(argc, argv);
   else {
     printf(ERROR_INVALID_COMMAND);	   
     showHelp(); 
   }
}

int getNumber(char* str) {
   int sum = 0;
   int len = strlen(str);
   for (int i = 0; i < len; i++) {
     sum = sum * 10 + str[i] - '0';
   }
   return sum;
}

void copyString(char* source, char* dest, int size) {
   for (int i = 0; i < size; i++) {
      dest[i] = source[i];
   }
}

int areSame(char* first, char* second, int length, int correctLength) {
   if (length != correctLength) return 0;

   for (int i = 0; i < length; i++) {
      if (first[i] != second[i]) {
         return 0;
      }
   }
   return 1;
}

void getParameterValues(char* param, int* type, int* num, int* position) {
   // type = 0 -> text
   // type = 1 -> numeric
   // type = 2 -> byte

   int fd = open(TABLE_FILE_NAME, O_RDONLY);
   int pos = 0, cnt, len;
   char ch, all[BUFF_SIZE], str[BUFF_SIZE];

   while (cnt = read(fd, &ch, 1)) {
      if (ch != '\n') all[pos++] = ch;
      else {
         len = pos;
         copyString(all, str, len - 3);
         if (areSame(str, param, strlen(param), len - 3) == 1) {
            *type = all[len - 3] - '0';
            *num = all[len - 2] - '0';
            *position = all[len - 1] - '0';
            close(fd);
            return;
         }
         pos = 0;
      }
   }
   close(fd);
}

void checkForBitChange(int flag, int fdWrite, int segNum, char bit, int position) {
   if (flag == 1) {
       char param = (char)(bit | (1 << (SEVENTH- position)));
       lseek(fdWrite, segNum * SEG_NUM + 1 , SEEK_SET);
       write(fdWrite, &param, 1);
   }
}

int checkForBitActivation(int fd, int position, int flag, char bit) {
   int k = SEVENTH- position;
   if (flag == 1 && (bit & (1 << k)) >> k != 1) {
      printf(ERROR_PARAMETER_NOT_ACTIVATED);
      close(fd);
      return 1;
   }
   return 0;
}

void initialiseData(int* fd, char* fileName, char* parameter, int* type, int* position, int* num) {
   *fd = open(fileName, O_RDONLY);
   if (*fd < 0) {
      printf(ERROR_FILE_DOES_NOT_EXIST, fileName);
      exit(1);
   }
   *type = INVALID_VALUE, *position = INVALID_VALUE, *num = INVALID_VALUE;
   getParameterValues(parameter, type, num, position);
   if (*type == INVALID_VALUE) {
      printf(ERROR_WRONG_PARAMETER);
      close(*fd);
      exit(1);
   }
}

int isValidSequence(int argc, char* argv[]) {
    int last = getNumber(argv[3]), newNum;
    if (last != 0) {
       printf(ERROR_INVALID_SEQUENCE);
       return 0;
    }
    for (int i = INT_SIZE; i < argc; i++) {
       if (strcmp(argv[i], "t") != 0 && strcmp(argv[i], "n") != 0 && strcmp(argv[i], "b") != 0) {
          newNum = getNumber(argv[i]);
          if (newNum != last + 1) {
             printf(ERROR_INVALID_SEQUENCE);
             return 0;
          } else last = newNum;
       }
    }
    return 1;
}

int matchRegex(int type, char* value) {
   int len = strlen(value);
   if (type == 1) {
      for (int i = 0; i < len; i++) {
         if (!((value[i] >= 'A' && value[i] <= 'Z') || (value[i] >= 'a' && value[i] <= 'z') || (value[i] >= '0' && value[i] <= '9') ||  value[i] == '_' || value[i] == '-' || value[i] == '.'))
            return 0;
      }
      return 1;
   }
   else if (type == 2) {
      for (int i = 0; i < len; i++) {
         if (!((value[i] >= 'A' && value[i] <= 'Z') || (value[i] >= '0' && value[i] <= '9')))
            return 0;
      }
      return 1;
   }
   else if (type == 3) {
      for (int i = 0; i < len; i++) {
         if (!((value[i] >= 'A' && value[i] <= 'Z') || (value[i] >= '0' && value[i] <= '9') || value[i] == ':'))
            return 0;
      }
      return 1;
   }
   else if (type == INT_SIZE) {
      for (int i = 0; i < len; i++) {
         if (!((value[i] >= 'a' && value[i] <= 'z') || (value[i] >= '0' && value[i] <= '9')))
            return 0;
      }
      return 1;
   }
   else if (type == FIFTH) {
      int number = getNumber(value);
      for (int i = 0; i < SIXTH; i++) {
         if (number == SERIAL_BAUDATE_VALUES[i]) return 1;
      }
      return 0;
   }
   else if (type == SIXTH) {
      int number = getNumber(value);
      for (int i = 0; i < SEVENTH; i++) {
         if (number == AUDIO_BITRATE_VALUES[i]) return 1;
      }
      return 0;
   }
   else if (type == SEVENTH) {
      int number = getNumber(value);
      for (int i = 0; i < FIFTH; i++) {
         if (number == SLEEP_PERIOD_VALUES[i]) return 1;
      }
      return 0;
   }
   else if (type == OFFSET) {
      for (int i = 0; i < INT_SIZE - 1; i++) {
         if (SERIAL_PARITY_VALUES[i] == value[0]) return 1;
      }
      return 0;
   }
   else if (type == 9) {
      int number = getNumber(value);
      return number >= FIFTH && number <= OFFSET;
   }
   else if (type == 10) {
      int number = getNumber(value);
      return number == 0 || number == 1;
   }
}

int isValidValue(int type, int num, int position, char* value) {
   if (type == 0 && num == 0 && (position == 0 || position == 1)) {
      return matchRegex(1, value);
   }
   else if (type == 0 && num == 0 && position == 2) {
      return matchRegex(2, value);
   }
   else if (type == 0 && num == 1 && (position == 0 || position == 1)) {
      return matchRegex(3, value);
   }
   else if (type == 0 && num == 1 && position == 2) {
      return matchRegex(INT_SIZE, value);
   }
   else if (type == 1 && num == 0) {
      if (position == 0) return matchRegex(FIFTH, value);
      else if (position == 1) return matchRegex(SIXTH, value);
      else return matchRegex(SEVENTH, value);
   }
   else if (type == 2 && num == 0) {
      if (position == 0) return matchRegex(OFFSET, value);
      else if (position == 1) return matchRegex(9, value);
      else return matchRegex(10, value);
   }
   return matchRegex(INT_SIZE, value);
}

void validateValue(int fd, int type, int num, int position, char* value) {
   if (!isValidValue(type, num, position, value)) {
      printf(ERROR_INVALID_VALUE, value);
      close(fd);
      exit(1);
   }
}

int isValidBit(char* bit) {
   return (strcmp(bit, "0") == 0 || strcmp(bit, "1") == 0) ? 1 : 0;
}
