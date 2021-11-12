#ifndef INTERPRETER_H
#define INTERPRETER_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "DISK_driver.h"
#include "shell.h"
#include "shellmemory.h"
#include "kernel.h"
#include "memorymanager.h"
int interpreter (char *arg1, char *arg2, char *arg3, char *arg4);
int help();
void quit();
int set (char *arg2, char *arg3);
int print (char *arg2);
int run (char *arg2);
void exec(char *f1, char *f2, char *f3);
// The following three function are added in HW4:
int mount (char *partitionName, char *number_of_blocks, char *block_size);
int writeFS (char *filename, char *words);
int readFS (char *filename, char *variable);
#endif

