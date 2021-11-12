#ifndef CPU_H
#define CPU_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shell.h"
#include "shell.h"
#include "pcb.h"
#include "memorymanager.h"

struct CPU {
	int IP;
	char IR[1000];
	int quanta;
	int offset;
};
int runCPU (int quanta);
int pageFault ();
#endif
