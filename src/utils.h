#ifndef UTILS_H
#define UTILS_H
#include <stdio.h>

void error(const char *s);
void not_implemented();
char *fgetl(FILE *fp);
char *copy_string(char *s);

#endif
