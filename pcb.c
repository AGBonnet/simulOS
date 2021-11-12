// Contains data-structures and helper functions for the process control blocks

#include "pcb.h"

// To create a new PCB using malloc
struct PCB* makePCB (){
	struct PCB* newPCB = (struct PCB*)malloc(sizeof(struct PCB));
	if (newPCB == NULL){
		printf("ERROR: Couldn't malloc a new PCB.\n");
		return NULL;
	}
	newPCB->PC = 0;	// PC initialized at start cell number
	for (int i=0; i<10; i++) {newPCB->pageTable[i] = -1;}
	newPCB->PC_page = 0;
	newPCB->PC_offset = 0;
	newPCB->pages_max = 0;
	return newPCB;
}
