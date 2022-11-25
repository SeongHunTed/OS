#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#define MAXPNUM 30
#define PAGENUM 500

int *randomPage();
int *readPage(FILE *fp, char *pageName);
int *readEscPage(FILE *fp, char *pageName);
int *writePage(FILE *fp);
int *escPage();
int user();