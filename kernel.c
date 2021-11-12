#include "kernel.h"


int main(){
	int err = 0;
	boot();
	int error = kernel();
	return error;
}

/* In the kernel, the boot() function does two things:
1. It assumes that RAM is a global array of 40 char* pointers. This array is not instantiated (not malloced). It assumes that each 4 cells of the array are a frame (i.e., the RAM can hold a total of 10 frames). At boot time, there are no other programs running except the kernel, so it initializes every cell of the array to NULL. This indicates that there are no pages of code in RAM.
2. The boot sequence also prepares the Backing Store. Typically, the Backing Store is a dedicated partition on the hard disk, but for us it will be simulated by a directory. Preparing the Backing Store means clearing it. Use the C system() command to delete the old backing store directory and then create a new directory. Name the directory BackingStore.   
*/
void boot(){
	printf ("Boot entered\n");
	// Clear the RAM
	memset(ram, 0, sizeof(ram));
	// Create a BackingStore directory (overwrite if already exists)
	system("rm -rf BackingStore; mkdir BackingStore");
	initIO();
}

int kernel(){
	printf("Kernel 3.0 loaded!\n");
	//Instantiate all the kernel data structures:
	ready = NULL;
	cpu.quanta = 2;
	cpu.IP = 0;
	shellUI();// Display command line
}
// To add the PCB to the tail of the ready queue. Each PCB is the ready queue
// is sorted First-in First-out (FIFO) and Round Robin (RR) in the same order
// as they appeared in the exec command
void addToReady (struct PCB *newPCB){
	struct readyQ* newQ = (struct readyQ*) malloc(sizeof(struct readyQ));
	newQ->node = newPCB;
	newQ->next = NULL;
	// if ready is NULL, set as ready, else append to the end. 
	if (!ready) {ready = newQ; return;}
	struct readyQ* temp = ready;
	while (temp->next) {temp = temp->next;}
	temp->next = newQ;
}

// Main execution loop
int scheduler(){
	while (ready){
		// Run 2 lines of code from the first PCB in ready queue
		struct PCB* p = ready->node;
		p->PC = 4 * p->pageTable[p->PC_page];	
		cpu.IP = p->PC;
		cpu.offset = p->PC_offset;
		int outcome = runCPU(cpu.quanta);

		// 0: Increment offset, put ready at the end of the queue
		if (outcome == 0) {p->PC_offset += 2; outcome = -2;}
		
		// -1: If next instruction null or program ends, terminate PCB
		else if (outcome == -1) {free(p); ready = ready->next;}
		
		// -2: Page fault, so we add the PCB to the end of the ready queue
		else if (outcome == -2 && ready->next) {
			struct readyQ* temp = ready;
                       	struct readyQ* temp2 = ready->next;  
			while (temp->next){temp = temp->next;}
                       	temp->next = ready;
                       	temp->next->next = NULL;
                       	ready = temp2;
		}
	}
	// Clear the RAM
        memset(ram,0,sizeof(ram));
	return 0; 
}
