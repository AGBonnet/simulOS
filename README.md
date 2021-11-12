# simulOS
A simple, simulated OS system in C, including kernel, shell, interpreter, basic memory structure, CPU, PCBs, paging and file systems.

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
