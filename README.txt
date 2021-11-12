COMP 310: Operating Systems Winter 2021
Assignment 4
Antoine Bonnet 260928321

(1) To compile this program:
	Type "bash bashfile" in the command line. 
	This will compile the code using gcc. 
(2) To run this program:
	- Option 1: Standard input by using "./mykernel" 
	- Option 2: Piping a file using "./mykernel < testfile.txt"

I have included the following files (and their repsective .h files):
	- kernel.c 		Main execution file
	- shell.c		Prompt + Command parser
	- Interpreter.c		Command interpreter
	- shellMemory.c		Data structures for the set and run commands
	- cpu.c			CPU simulation
	- pcb.c			PCB Data structure for exec command 
	- memorymanager.c	Launch paging technique for exec command
	- DISK_driver.c		Simulates a file system

My testfile is named "TESTFILE.txt".

NOTE: I'm pretty sure my file system is non-contiguous, because I can do the following:
	write file1 [test1]
	write file2 [test2]
	write file1 [test3]
and the partition data area looks like: test1-test2-test3, where test1 and test3 belong
to the same file file1, and so the file system is non-contiguous. Might have to check though!
