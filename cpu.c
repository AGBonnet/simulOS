// CPU.C FILE 			Antoine Bonnet 260928321 March 2021
#include "cpu.h"

// Contains data-structures and algorithms to simulate task switching
/* In addition to switching tasks when the quanta expires, our CPU now needs to handle page faults. A page fault occurs when we run out of lines of code in our frame (each frame stores pointers to 4 lines of code).
Important: When the offset reaches 4, generate a pseudo-interrupt: regardless at what quanta count the program is at, execution stops because the CPU is at the end of the frame, and the page fault operation must happen. */

int runCPU (int quanta){
	while (quanta > 0){
                struct PCB *p = ready->node;
                if (!p) {return 0;}
		// Copy the line of code at address in cpu.IR	
		int address = cpu.IP + cpu.offset;
		if (!ram[address]) {return -1;}

		// Clear cpu.IR then copy ram[address] into it
		memset(cpu.IR, 0, sizeof(cpu.IR));
		strcpy(cpu.IR, ram[address]); 

		// Run the command:
		char *args[4];
		if (parse(args, cpu.IR)>0) {continue;}
		interpreter(args[0], args[1], args[2], args[3]);
		
		// PAGE FAULT: If we run out of lines in the frame
		cpu.offset++;
                if (cpu.offset == 4) { 
			pageFault(); 
			if (ready->node->PC_page > ready->node->pages_max) { return -1; }
			else { return -2; }
		}
		quanta--;
	}
        if (!ram[cpu.IP + cpu.offset]) {return -1;}
	return 0;
}

// PAGE FAULT HELPER FUNCTION: If we run out of lines in the frame
int pageFault() {
	struct PCB *p = ready->node;
	// 1. Determine the next page
	p->PC_page++;
	
	// 2. if page > pages_max, terminate
        if (p->PC_page > p->pages_max) {return 0;}
        
	// 3. Check if frame for that page exists in the pageTable
        int frame = p->pageTable[p->PC_page];
        if (frame >= 0){p->PC = 4*frame; p->PC_offset = 0;}
	
	// 4. Else, find page on disk & update PCB page table(s)
        else {  // a. Find page in backingstore
		FILE *fp = fopen(p->uid, "r"); 
                
		// b. Find space in RAM (free cell or select victim)
                int free = findFrame();
                int victim = -1;
                if (free < 0) {victim = findVictim(p);}

                // c. Update page table(s)
                updatePageTable(p, p->PC_page, free, victim);

                // d. Update the RAM frame with instructions
                if (victim >= 0) {free = victim;}
                loadPage(p->PC_page, fp, free);
		
                // e. Reset PC and PC_offset fields
                p->PC = 4*free;
                p->PC_offset = 0;
	}
	return 1;
}
