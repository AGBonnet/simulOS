/* DISK DRIVER PROGRAM Antoine Bonnet 260928321 April 2021 */
#include "DISK_driver.h"

// ===================== DATA STRUCTURES =======================

/* The PARTITION structure records information about the format of the partition. 
   A disk partition is composed of two areas: the partition header and the partition
   data area. The partition header contains the PARTITION structure 
   and the File Allocation Table (FAT) structure */

struct PARTITION {
        int total_blocks;
        int block_size;
	char *name;
} partition;

struct FAT {
        char *filename; // name of a file in the partition.
        int file_length; // length of the file in number of blocks
        // pointer to the current read/write location of the file in block number.
        // It is initialized to -1 when not used.
        int current_location;
        //  block numbers in the partition populated with data from this file.
        int blockPtrs[10];
} fat[20];

/* The active file table contains all the system wide open file pointers, which is 5 maximum.
 * This is a private structure but global within DISK_driver.c. */
FILE *active_file_table[5];

// Data structure to remember which active_file_table entries belong to which fat[].
int activeToFAT[5];     // ActiveToFAT[i] = index in FAT[20] of active_file_table[i]


// ================== FUNCTIONS ===========================

/* Initialize all global data structure and variables to zero or null.
 * Called from your boot() function. */
void initIO () {
	partition.total_blocks = 0;
	partition.block_size = 0;
	partition.name = NULL;
	for (int i=0; i<20; i++) { 
		fat[i].filename = "-";
		fat[i].file_length = 0;
		fat[i].current_location = 0;
		for (int j=0; j<10; j++) { fat[i].blockPtrs[j] = -1; }
	}
	for (int i=0; i<5; i++) {
		active_file_table[i] = NULL;
		activeToFAT[i] = 0;	
	}
}

/* Create & format partition. Called from your mount() function that 
 * lives in the interpreter, associated to your scripting mount command. */
int partitionFS (char *path, char *partitionName, int blocksize, int totalblocks, char *data) {
	// Initialize partition struct
	partition.block_size = blocksize;
	partition.total_blocks = totalblocks;
	partition.name = strdup (partitionName);
	FILE *file = fopen(path, "w");

	// Each partition is simulated by one file. The simulated files 
	// within the partition are stored as the contents of the partition file.
	// beginning of the file contains the information from struct partition
	char buffer[4000];
	char line[200];
	memset (buffer, 0, 4000);
	sprintf (line, "%d\n%d\n", blocksize, totalblocks); 
	strcat (buffer, line);

	// Store information from fat[20] (initially empty)
	for (int i=0; i<20; i++) {
		sprintf (line, "%s ", fat[i].filename); strcat (buffer, line);
		sprintf (line, "%d ", fat[i].file_length); strcat (buffer, line);
		sprintf (line, "%d ", fat[i].current_location); strcat (buffer, line);
		for (int j=0; j<9; j++) {
			sprintf (line, "%d,", fat[i].blockPtrs[j]); strcat (buffer, line);
		}
		sprintf (line, "%d\n", fat[i].blockPtrs[9]);
		strcat (buffer, line);
	}

	// if data argument is NULL: append the partition data area by writing 
        // total blocks * blocksize number of '0' characters to EOF
	if (!data) {for (int i=0; i<blocksize * totalblocks; i++) { strcat (buffer, "0"); }}
	// If data argument is not NULL, append partition data area
	else {strcat (buffer, data);}
	
	// Store entire partition info into file
	fputs (buffer, file);
	fclose (file);
	return 1;
}

/*  Load FAT & create buffer_block. Called from your mount() function that 
 *  lives in the interpreter, associated to your scripting mount command. */
int mountFS (char *path) {
	FILE *file = fopen (path, "r");
	// Load data from partition into global structures partition and fat[].
	char line [1000];
	fgets (line, sizeof(line), file);
	line[strcspn (line, "\n")] = 0;
	partition.block_size = atoi (line);
	fgets (line, sizeof(line), file);
	line[strcspn (line, "\n")] = 0;
	partition.total_blocks = atoi (line);
	// LOAD FAT CONTENT
	for (int i = 0; i<20; i++) {
		fgets (line, sizeof(line), file);
		line[strcspn (line, "\n")] = 0;
		fat[i].filename = strdup (strtok (line, " "));
		fat[i].file_length = atoi (strtok (NULL, " "));
		fat[i].current_location = atoi (strtok (NULL, " "));
		for (int j=0; j<10; j++) { 
			fat[i].blockPtrs[j] = atoi (strtok (NULL, ","));
		}
	}
	fclose (file);
	return 1;
}

/* Find filename or creates file if it does not exist, returns file’s FAT index
 * Called from your scripting read and write commands in the interpreter. */
// MODE: 0 for read, 1 for write
int openfile (int mode, char *fileName) {
	if (!partition.name) { return -3; }
	// Create file pointer
        char path[100];                                
	sprintf (path, "PARTITION/%s", partition.name);                 
	FILE *ptr = fopen (path, "r+"); 		
	if (!ptr) {return -1;}

	// Check there is enough space in PDA                      
	if (findSpace (ptr) == -1) {return -1;}

	// Assume the FAT contains data. Search for the string name argument
	// in the filename fields of fat[20].
	for (int i=0; i<20; i++) {
		// If it finds the file: Available cell in active file table
		// points to 1st block of file
		if (strcmp (fileName, fat[i].filename) == 0) {return i;}
	}
	if (mode == 0) {return -2;} // if read and no match, failure
	// Otherwise, create new entry in FAT, leave the corresponding
	// active file table cell NULL and return FAT index of that new entry.
	for (int i=0; i<20; i++) {
		if (strcmp (fat[i].filename, "-") == 0) {
			strcpy (fat[i].filename, fileName);
			// Find empty AFT cell, leave it  NULL and update activeToFAT
			for (int j=0; j<5; j++) {
				if (! active_file_table [j]) {
				       	// Advance file ptr to 1st block pointer
					int firstBlock = findSpace (ptr);
        				advancePointer (ptr, firstBlock);
					active_file_table[j] = ptr;
					activeToFAT[j] = i;
					return i; // return FAT index to new entry
				}
			}
			fclose (ptr);
			return -1; // no AFT free cell
		}	
	}
	fclose (ptr);
	return -1; // Error: no match or no available cell
}

/* Using the file FAT index number, return data from file */
char *readBlock (int file) {
	// Find file in AFT
	int index = findFile (file);
	if (index < 0) {return NULL;}
	FILE *fp = active_file_table[index];

	// if at EOF, failure 
	if (feof (fp)) { active_file_table[index] = NULL;
		fclose (fp); return NULL; }

	// For each block pointer, load data at given block and store it
	char buffer[partition.block_size * 10 + 1];
	memset (buffer, 0, sizeof (buffer));
	char temp[partition.block_size+1];
	memset (temp, 0, sizeof (temp));
	for (int i = 0; i < 10; i++) {
		int block = fat[index].blockPtrs[i];
		if (block < 0) {break;}
		advancePointer (fp, block);
		fread (temp, partition.block_size, 1, fp);
		int j = partition.block_size - 1;
		while (temp[j] == 48) {temp[j] = '\0'; j--;}
		strcat (buffer, temp);
	}
	return strdup (buffer);
}

int writeBlock (int file, char *data) {
	// Find file pointer in AFT
        int index = findFile (file);
        if (index < 0) {return index;}
        FILE *fp = active_file_table[index];

	// Find number of blocks needed to store data	
        int bSize = partition.block_size;
        int size = bSize * partition.total_blocks;
        int length = strlen (data);
        int remainder = length % bSize;
        int numBlocks;
        if (remainder == 0) {numBlocks = length / bSize;}
        else {numBlocks = length / bSize + 1;}
       	if (numBlocks > 10) {return 0;}

	// Duplicate data and fill extra space with 0's
	char temp [numBlocks * bSize];
	strcpy (temp, data);
	for (int j = 0; j < bSize - remainder; j++) {strncat (temp, "0", 1);}
	
	// Break up data into blocks
	char * dataBlocks[numBlocks];
	for (int i = 0; i < numBlocks; i++) {
		dataBlocks[i] = (char *) malloc (bSize + 1);
		strncpy (dataBlocks[i], &temp[i*bSize], bSize);
	}
	
	// Get initial partition data
        advancePointer (fp, 0);
        char line[size];
        fgets (line, sizeof(line), fp);

	// Get first available cell in PDA
        int blockIndex = findSpace (fp);
	
	// Check there is enough space in partition data area
        if ((blockIndex + numBlocks) * bSize >= size) {return 0;}
        for (int k = blockIndex * bSize; k < (numBlocks + blockIndex) * bSize; k++) {
                if (line[k] != 48) {return 0;}
        }
	
	// Find first available block pointer in FAT
	int ptrIndex = 0; 
        while (fat[file].blockPtrs[ptrIndex] >= 0) {ptrIndex++;}

	// Overwrite each block and update FAT info every time
	for (int i = 0; (i < numBlocks) && (ptrIndex < 10); i++) {
        	fat[file].blockPtrs[ptrIndex] = blockIndex;
		advancePointer (fp, blockIndex);
		fwrite (dataBlocks[i], 1, bSize, fp);
		blockIndex++;	ptrIndex++;
		fat[file].file_length += numBlocks * bSize;
        }
	fat[file].current_location = blockIndex;
	
	// Store updated partition data area
        advancePointer (fp, 0);
	char *PDA = (char *) malloc (size + 1);
        fgets (PDA, size, fp);
	fclose (fp);	

	// Overwrite partition file
	char path[200];
	sprintf (path, "PARTITION/%s", partition.name);
	partitionFS (path, partition.name, partition.block_size, partition.total_blocks, PDA);
	active_file_table[file] = fopen(path, "r+");
	return 1;
}

// ===================== HELPER FUNCTIONS =====================

// Helper function: find index of file
int findFile (int file) {
	// Find current location of fat[file]
        if (file >= 20 || file < 0) {return -1;}
        // Find which file pointer to use through the AFT entry
        for (int i=0; i<5; i++) {
                if (activeToFAT[i] == file) {return i;}
        }
	return -1;
}

// Helper function: advance file pointer to point at given block index
void advancePointer (FILE *fp, int blockIndex) {
	if (blockIndex > partition.total_blocks - 1) {return;}
	char skip [1000];
	fseek (fp, 0, 0);
	for (int i=0; i<22; i++) {
		fgets (skip, sizeof(skip), fp);
	}
	int offset = partition.block_size * blockIndex;
	fseek (fp, offset, SEEK_CUR);
}

// Helper function: returns the block index of the first free block in PDA
// returns -1 if no space available
int findSpace (FILE * fp) {
	advancePointer (fp, 0);
	char line [partition.block_size * partition.total_blocks + 1];
	fgets (line, sizeof(line), fp);
	rewind (fp);
	for (int i = 0; i < partition.total_blocks; i++) {
		if (line[partition.block_size * i] == 48) {
			return i;
		}
	}
	return -1;
}
