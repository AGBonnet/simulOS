#ifndef DISKDRIVER_H
#define DISKDRIVER_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
void initIO ();
int partitionFS (char *path, char *partitionName, int blocksize, int totalblocks, char *data);
int mountFS (char *path);
int openfile (int mode, char *fileName);
char *readBlock (int file);
int writeBlock (int file, char *data);

// HELPER FUNCTIONS:
void advancePointer (FILE *fp, int blockIndex);
int findFile (int file);
int findSpace (FILE * fp);
#endif

