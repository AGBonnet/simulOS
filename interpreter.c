#include "interpreter.h"

// Each command the interpreter() function accepts has a corresponding function that implements the command’s functionality
// Checks if the input is of the correct form then calls the appropriate function or prints an error.
int interpreter (char *arg1, char *arg2, char *arg3, char *arg4) {
	if (!arg1) {return 2;}
	if ((strcmp (arg1, "help") == 0) && !arg2 && !arg3 && !arg4) 
		{help ();}
	else if ((strcmp (arg1, "quit") == 0) && !arg2 && !arg3 && !arg4) 
		{quit ();}
        else if ((strcmp (arg1, "set") == 0) && arg2 && arg3 && !arg4) 
		{set (arg2, arg3);}
        else if ((strcmp (arg1, "print") == 0) && arg2 && !arg3 && !arg4) 
		{print (arg2);}
        else if ((strcmp (arg1, "run") == 0) && arg2 && !arg3 && !arg4) 
		{run (arg2);}
	else if ((strcmp (arg1, "exec") == 0) && arg2) 
		{exec (arg2, arg3, arg4);}
	else if ((strcmp (arg1, "mount") == 0) && arg2 && arg3 && arg4) 
		{if (mount (arg2, arg3, arg4)==0) {
			printf ("Error. There was an issue with this mount command\n");}}	  
	else if ((strcmp (arg1, "write") == 0) && arg2 && arg3) {
		int err = writeFS (arg2, arg3);
		if (err == -1) {
			printf ("Error. There was an issue with this write command\n");
		}
		else if (err == -2) {
			printf ("Error. You cannot write to a file without first mounting a partition\n");
		}
	}
	else if ((strcmp (arg1, "read") == 0) && arg2 && arg3) {
		       	int err = readFS (arg2, arg3);
			if (err == 0) {
			printf ("Error. There was an issue with this read command\n");}
		 	else if (err == -1) {
				printf ("Error. The file %s does not exist\n", arg2);}
			else if (err == -2) {
				printf ("Error. You cannot read a file without first mounting a partition\n");
			}
	}
	else {printf("Unknown command. Look for commands by typing 'help'.\n"); return 1;}
	return 0;

}

// help: Displays all the commands. 
int help () {
	printf("\nCommand\t\t\tDescription\nhelp\t\t\tDisplays all the commands\n");
	printf("quit\t\t\tExits/terminates the shell with 'Bye!'\n");
	printf("set VAR STRING\t\tAssigns a value to shell memory\n");
	printf("print VAR\t\tDisplays the STRING assigned to VAR\n");
	printf("run SCRIPT.TXT\t\tExecutes the file SCRIPT.TXT\n");
	printf("exec SCRIPT.TXT\t\tExecutes the file SCRIPT.TXT\n");
	printf("mount PARTITION_NAME NUMBER_OF_BLOCKS BLOCK_SIZE\t\tCreates a new partition of files\n");
	printf("write FILENAME [WORDS]\t\tWrites WORDS to file FILENAME\n");
	printf("read FILENAME VARIABLE\t\tReads file FILENAME and stores its content in VARIABLE\n\n");
	return 0;
}

// quit: Exits/Terminates the shell with "Bye!" 
void quit () { printf("Bye!\n"); exit(0); }

/* set VAR STRING: Assigns a value to shell memory.
 First checks to see if VAR already exists. If it does, STRING overwrites the previous value of VAR. 
 If not, a new entry is added to the shell memory where the variable name is VAR and the contents of the variable is STRING. */
int set (char *arg2, char *arg3) {
	for (int i=0; i<1000; i++){// If there is a match, replace value.
		if (memArray[i].var == NULL) {
			memArray[i].var = (char*)malloc(sizeof(arg2));
			strcpy(memArray[i].var, arg2);
			memArray[i].value = (char*)malloc(sizeof(arg3));
			strcpy(memArray[i].value, arg3);
			return 0;
		}
		else if (strcmp(memArray[i].var, arg2) == 0){
			strcpy(memArray[i].value, arg3);
			return 0;
		}	
	}
	return 2;
}

/* print VAR: Displays the STRING assigned to VAR.
 First checks to see if VAR exists. If not, displays error message. 
 If VAR does exist, then it displays the STRING. */
int print (char *arg2) {
	for (int i=0; i<1000; i++){
		// If we have reached the end of the filled array without match 
               	if (memArray[i].var == NULL) {
                        printf("Variable %s does not exist.\n", arg2);
                        return 0;
                }	
		// If there is a match, print value.
		else if (strcmp(memArray[i].var, arg2) == 0) {
                        printf("%s\n", memArray[i].value);
                        return 0;
		}
	}
	return 3;
}

/* run SCRIPT.TXT: Executes the file SCRIPT.TXT
 Assumes that a text file exists with the provided file name. 
 It opens that text file and then sends each line one at a time to the interpreter. 
 The intepreter treats each line of text as a command. Each line affects the shell and the UI. 
 At the end of the script, the file is closed and the command line prompt is displayed once more.
 While the script executes the command line prompt is not displayed. */
int run (char * arg2) {
	FILE *fp = fopen(arg2, "r");
	// If file not found: print error
	if (fp == NULL) { printf("Script not found.\n"); return 4; }
	// read SCRIPT.txt line by line, recursively call intepreter() on each line.  
	char entry[999];
	while (fgets(entry, sizeof(entry), fp)) {
		char *args[3];
		if (strcmp(entry, "\n") == 0){continue;}
		if (parse(args, entry)>0){fclose(fp); return 1;}
		if (interpreter(args[0], args[1], args[2], args[3])>0){fclose(fp); return 1;}
		// If the above line caused an error, stop the execution of SCRIPT.TXT
	}
	fclose(fp);
	return 0;
}

/* Interpreter function that does the following: 
• It handles the filename argument verification error.
• It opens each program file.
• It calls the myinit() function in kernel.c (see below) to load each program into the
simulation. For example, if there are three programs then myinit() is called three times.
• Only if myinit() is successful, it starts the execution of all the loaded programs by calling  the scheduler() function in kernel.c.
• The exec() function does not terminate until all the programs it loaded have all completed execution.

*/
void exec(char *f1, char *f2, char *f3) {
	int n = 0;
	if (f1) {if (launcher(f1)==0) {n++;}}
	if (f2) {if (launcher(f2)==0) {n++;}}
	if (f3) {if (launcher(f3)==0) {n++;}}
	if (n == 0){scheduler();}
}

// The following three function are added in HW4:

/* The command mount uses the partition() and mountFS() functions, described below. 
 * If a partition already exists with the name provided then the existing partition 
 * is mounted, otherwise the provided arguments are used to create and mount the new partition. 
 * All the arguments are required. */
int mount (char *partitionName, char *number_of_blocks, char *block_size) {
	initIO ();
	int blocksize = atoi (block_size);
	int totalblocks = atoi (number_of_blocks);
	// To simulate a partition, we will use a subdirectory PARTITION:
        system ("mkdir -p PARTITION");
        // Within PARTITION create a file having the name argument as its filename. 
        char path[200];
        sprintf (path, "PARTITION/%s", partitionName);	
	partitionFS (path, strdup (partitionName), blocksize, totalblocks, NULL);
	mountFS (path);
	return 1;
}


// Write command: Check if filename is open (if not, open a new file). 
// Write words to file in last mounted partition.

int writeFS (char *filename, char *words) {
	// Check if filename is open.
	int index = openfile (1, filename);
	if (index < 0) {return 0;} // if file couldn't be opened
	else if (index == -3) {return -1;} // no partition
	return writeBlock (index, words);	
}

// Read command: Check if filename is open (if not, return an error)
//
int readFS (char *filename, char *variable) {
	// Check that filename is open.
	int index = openfile (0, filename);
	if (index == -2) {return -1;} // file doesn't exist 
	else if (index == -3) {return -2;} // no partition
	char *readValue = readBlock (index);
	if (!readValue) {return 0;}
	// Call interpreter to set variable to this file content	
	interpreter ("set", variable, readValue, NULL);	
	return 1;
}

