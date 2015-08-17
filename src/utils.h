#ifndef UTILS_H
#define UTILS_H
#include <stdio.h>

void file_error(char *s);
void error(const char *s);
void not_implemented();
char *fgetl(FILE *fp);
char *append_string(char *s, char *after);
char *copy_string(char *s);

#endif
