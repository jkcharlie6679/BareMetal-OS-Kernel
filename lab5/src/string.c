#include "printf.h"
#include "string.h"

int strlen(char *s){
  int i = 0;
  while(s[i] != '\0'){
    i++;
  }
  return i;
}

int atoi(char* str){
  int res = 0;
  for (int i = 0; str[i] != '\0'; ++i)
    res = res * 10 + str[i] - '0';
  return res;
}

void append_str(char *s, char ch){
  int i = strlen(s);
  *(s+i) = ch;
  *(s+i+1) = 0;
}

void pop_str(char *s){
  int i = strlen(s);
  s[i-1] = 0;
}

int strcmp(const char *X, const char *Y){
  while (*X){
    if (*X != *Y) {
      break;
    }
    X++;
    Y++;
  }
  return *(const unsigned char*)X - *(const unsigned char*)Y;
}

int myHex2Int(char* str){
  int res = 0;
  for (int i = 0; str[i] != '\0'; ++i){
    if(str[i] > 0x60 && str[i] < 0x67){
      res = res * 16 + str[i] - 0x57;
    }else if(str[i] > 0x40 && str[i] < 0x47){
      res = res * 16 + str[i] - 0x37;
    }else{
      res = res * 16 + str[i] - '0';
    }
  }
  return res;
}

// Credits to: https://stackoverflow.com/questions/28931379/implementation-of-strtok-function, Shirley V
char *strtok(char *str, char *delimiter){
  static char *temp_ptr = NULL;
  char *final_ptr = NULL;
  /* Flag has been defined as static to avoid the parent function loop  *
   * runs infinitely after reaching end of string.                      */ 
  static int flag = 0;
  int i, j;

  if (delimiter == NULL) {
    flag = 0;
    temp_ptr = NULL;
    return NULL;
  }

  /* If flag is 1, it will denote the end of the string */
  if (flag == 1) {
    flag = 0;
    temp_ptr = NULL;
    return NULL;
  }

  /* The below condition is to avoid temp_ptr is getting assigned   *
   * with NULL string from the parent function main. Without        *
   * the below condition, NULL will be assigned to temp_ptr         *
   * and final_ptr, so accessing these pointers will lead to        *
   * segmentation fault.                                            */
  if (str != NULL) { 
    temp_ptr = str; 
  }

  /* Before function call ends, temp_ptr should move to the next char,      *
   * so we can't return the temp_ptr. Hence, we introduced a new pointer    *
   * final_ptr which points to temp_ptr.                                    */
  final_ptr = temp_ptr;

  for (i = 0; i <= strlen(temp_ptr); i++)
  {
    for (j = 0; j < strlen(delimiter); j++) {

      if (temp_ptr[i] == '\0') {
        /* If the flag is not set, both the temp_ptr and flag_ptr   *
         * will be holding string "Jasmine" which will make parent  *
         * to call this function strtok infinitely.                 */
        flag = 1;
        if(final_ptr == NULL){
          flag = 0;
          temp_ptr = NULL;
        }
        return final_ptr;
      }

      if ((temp_ptr[i] == delimiter[j])) {
        /* NULL character is assigned, so that final_ptr will return    *
         * til NULL character. Here, final_ptr points to temp_ptr.      */
        temp_ptr[i] = '\0';
        temp_ptr += i+1;
        if(final_ptr == NULL){
          flag = 0;
          temp_ptr = NULL;
        }
        return final_ptr;
      }
    }
  }
  /* You will somehow end up here if for loop condition fails.    *
   * If this function doesn't return any char pointer, it will    *
   * lead to segmentation fault in the parent function.           */
  flag = 0;
  temp_ptr = NULL;
  return NULL;
}

int spilt_strings(char** str_arr, char* str, char* deli){
  int count = 0;
  // Spilt str by specified delimeter
  str_arr[0] = strtok(str, deli);
  count = 0;
  while(str_arr[count] != NULL){
    count++;
    str_arr[count] = strtok(NULL, deli);
  }
  return count;
}
