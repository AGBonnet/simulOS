#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H
#include "kernel.h"
#include "pcb.h"
int launcher (char* filename);
char* createPath (char* filename);
int countTotalPages (FILE *p);
void loadPage (int pageNumber, FILE *f, int frameNumber);
int findFrame();
int findVictim();
int updatePageTable (struct PCB *p, int pageNumber, int frameNumber, int victimFrame);


#endif
