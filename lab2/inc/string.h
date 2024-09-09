#ifndef STRING_H
#define STRING_H

#ifdef __cplusplus
extern "C" {
#endif

int strlen(char *s);
void append_str(char *s, char ch);
void pop_str(char *s);
int strcmp(const char *X, const char *Y);
int myHex2Int(char* str);

#ifdef __cplusplus
}
#endif

#endif
