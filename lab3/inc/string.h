#ifndef STRING_H
#define STRING_H

#ifdef __cplusplus
extern "C" {
#endif

int strlen(char *s);
void append_str(char *s, char ch);
int atoi(char* str);
void pop_str(char *s);
int strcmp(const char *X, const char *Y);
int myHex2Int(char* str);
char *strtok(char *str, char *delimiter);
int spilt_strings(char **str_arr, char *str, char *deli);

#ifdef __cplusplus
}
#endif

#endif
