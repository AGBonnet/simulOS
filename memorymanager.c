// MEMORY MANAGER FILE
// Antoine Bonnet 260928321 March 2021

#include "memorymanager.h"

/*The launcher() function returns a 1 if it was successful launching the program,
 * otherwise it returns 0. Launching a program consists of the following steps:
1. Copy the entire file into the Backing Store.
2. Close the file pointer pointing to the original file.
3. Open the file in the Backing Store.
4. Our launch paging technique loads two pages of the program into RAM when it is first launched.
A page is 4 lines of code. 
If the program has 4 or less lines of code, then only one page is loaded.
If the program has more than 8 lines of code, then only the first two pages are loaded. */

int launcher (char* filename){
	// 1. Copy the entire file into the Backing Store
	FILE *oldFile = fopen(filename, "r");
        if (!oldFile){
                printf("ERROR: Could not open the file %s.\n", filename);
                return 0;
        }
	char *path = createPath(filename);
	FILE *newFile = fopen(path, "w");
	char c = fgetc(oldFile);
	while (c != EOF) {fputc(c, newFile); c = fgetc(oldFile);}

	// 2. Close the file pointer pointing to the original file
	fclose(oldFile);
	fclose(newFile);
	// 3. Open the file in the BackingStore
	FILE *file = fopen(path, "r");
	
	// Construct PCB and add to Ready List
	struct PCB* p = makePCB();
	p->uid = path;
	addToReady(p);

	// 4. Launch Paging:
	int count = countTotalPages(file);
	if (count < 0) {return 0;}
        p->pages_max = count-1; // pages_max = max index of page
	
	// If ≤4 lines of code, load one page. Else, load first 2 pages.
	int numPages;
	if (count <= 1) {numPages = 1;}
	else {numPages = 2;}
	for (int page = 0; page < numPages; page++) {
		rewind(file); // Reset pointer to the start of the file
		// Find a free frame:
		int frame = findFrame();
		int victimFrame = -1;

		if (frame >= 0) {// Free frame
			loadPage(page, file, frame);
		}
		else {// If no free frame, select a victim		
			victimFrame = findVictim(p);
			if (victimFrame < 0) {return 0;}
			loadPage(page, file, victimFrame);
		}
		if (updatePageTable(p, page, frame, victimFrame) == 0) {return 0;}
	}
	return 1;
}

// Helper function to construt a unique filename in BackingStore
char* createPath (char* filename) {
        // a. Remove the .txt extension:
        char *name = (char *) malloc(100);
        strcpy(name, filename);
        int size = strlen(name);
        name[4 <= size ? size-4 : 0] = '\0';
        
	// b. Count number of files in BackingStore
        int numFile;
        FILE *q = popen("ls BackingStore | wc -l", "r");
        fscanf(q, "%d", &numFile);
        pclose(q);
        numFile++;

        // c. Construct unique path: /BackingStore/filename<num>.txt
        char *path = (char *) malloc(100);
        sprintf(path, "BackingStore/%s-%d.txt", name, numFile);
	return path;
}

/* This function returns the total number of pages needed by the program.
 * For example, if the program has L lines or less of code, 
 * if L < 4 the function returns 1. If 4 < L ≤ 8, it returns 2, etc.*/

int countTotalPages (FILE *file){
	rewind(file);
	// Count number of newline characters in p
	int Lines = 0;
	char c = fgetc(file);
        while (c != EOF) { if (c == '\n') {Lines++;}; c = fgetc(file);
	}
	// Total number of pages needed by p
	for (int n = 4; n < 40; n += 4) {if (n >= Lines) {return n/4;}}
	return -1;
}

/* FILE *f points to the beginning of the file in the Backing Store.
 * The variable page Number is the desired page from the Backing Store. 
 * The function loads the 4 lines of code from the page into the frame in ram[]. */

void loadPage (int pageNumber, FILE *f, int frameNumber){
	rewind(f);
	// Skip the lines before the desired page
	char line[400];
	for (int n = 0; n < 4*pageNumber; n += 1){
		if (feof(f)) {return;}
		fgets(line, sizeof(line), f);
	}
	// Transfer the 4 lines in the given page to ram
	for (int i = 0; i<4; i++) {
		if (fgets(line, sizeof(line), f)) {
                        ram[4*frameNumber + i] = strdup(line);
                }
                else {ram[4*frameNumber+i] = NULL;}
	}
}

/* Use the FIFO technique to search ram[] for a free frame. 
 * If one exists then return its index number, otherwise return -1.*/

int findFrame(){
	for (int i = 0; i<10; i++) {
		if (!ram[4*i]) {
			ram[4*i+1] = NULL;
			ram[4*i+2] = NULL;
			ram[4*i+3] = NULL;
			return i;
		}
	}
	return -1;
}

/* This function is only invoked when findFrame() returns a -1. 
 * Use a random number generator to pick a frame number. If the frame number does 
 * not belong to the pages of the active PCB (i.e., it is not in its page table) 
 * then return that frame number as the victim, otherwise, starting from the 
 * randomly selected frame, iteratively increment the frame number (modulo-wise) until 
 * you come to a frame number not belonging to the PCB’s pages, and return that number.*/

int findVictim (struct PCB *p){
	// Generate a random frame index between 0 and 9:
	int index = rand() % 10;
	int belongs = 0;
	for (int j=0; j<10; j++){
		int belongs = 0;
		int frame = index + j % 10;
		// Check that the frame is not in the PC
		for (int i=0; i<10; i++){
			if (p->pageTable[i] == frame) {belongs = 1;}
		}
		if (belongs == 0) {return frame;}
	}
	return -1;
}

/* The page table must also be updated to reflect the changes. 
 * We do this once for the PCB asking for the page fault, and we might do it again 
 * for the victim PCB (if there was one). If a victim was selected, then the PCB 
 * page table of the victim must be updated. 
 * p-> pageTable[pageNumber] = frameNumber (or = victimFrame). */

int updatePageTable (struct PCB *p, int pageNumber, int frameNumber, int victimFrame){
	if (frameNumber >= 0) {
		p->pageTable[pageNumber] = frameNumber;
		return 1;
	}
	else if (victimFrame >= 0) {
		p->pageTable[pageNumber] = victimFrame;
		//update the victim PCB:
		struct readyQ *ptr = ready;
		while (ptr){
			for (int i=0; i < 10; i++){
				if (ptr->node->pageTable[i] == victimFrame) {
					ptr->node->pageTable[i] = -1;
					return 1;
				}
			}
			ptr = ptr->next;
		}
	}
	return 0;
}
