int strlen (char *s) {
  int i = 0;
  while(s[i] != '\0')
    i++;
  return i;
}

void append_str (char *s, char ch) {
  int i = strlen(s);
  s[i] = ch;
  s[i + 1] = 0; // end of string
}

void pop_str (char *s) {
  int i = strlen(s);
  s[i - 1] = 0;
}

int strcmp(const char *X, const char *Y) {
  while (*X) {
    if (*X != *Y)
      break;
    X++;
    Y++;
  }
  return *(const unsigned char*)X - *(const unsigned char*)Y;
}
