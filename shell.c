#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shell.h"
#include <unistd.h>

int shellUI (){
	printf("Welcome to Antoine Bonnet's Shell! Version 4.0 Updated April 2021\n");
	prompt();
	if (!isatty(fileno(stdin))){
		if (freopen("/dev/tty", "r", stdin)){prompt();}
	}
	return 0;
}
int prompt(){
        char* arguments[4];
	while (!feof(stdin)) {
		if (isatty(fileno(stdin))){printf("$ ");}
		arguments[0] = NULL; arguments[1] = NULL; arguments[2] = NULL; arguments[3] = NULL;
                // read user input line by line, and parse line
		char line[999];
		memset(line, 0, sizeof(line));
		fgets(line, sizeof(line), stdin);
		if (parse(arguments, line)<0) {
			printf ("Invalid command. Check the syntax with 'help'.\n");
			continue;
		}
		interpreter(arguments[0], arguments[1], arguments[2], arguments[3]);
	}
        return 0;
}

// Takes as input a string LINE cmd[Space]arg1[Space]arg2\n, stores the arguments into the array arguments[]
int parse (char *arguments[], char line[]) {
	if (!line || strcmp(line, "\n") == 0) {return 1;}
	// Remove trailing newline and separate using spaces as delimiters
	// "write filename [collection of words]"
	strtok(line, "\n");
	arguments[0] = strtok(line, " ");
	arguments[1] = strtok(NULL, " ");
	if (strcmp (arguments[0], "write") == 0) {
		char *words = strtok (NULL, "]");
		if (!words || words[0] != '[') {return -1;}
		arguments[2] = words+1;
		arguments[3] = NULL;
	}
	else {
		arguments[2] = strtok(NULL, " ");
		arguments[3] = strtok(NULL, " ");
	}
	return 0;
}
