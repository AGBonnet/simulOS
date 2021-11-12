#ifndef SHELL_H
#define SHELL_H

#include "interpreter.h"
int shellUI();
int parse(char *arguments[], char line[]);
int prompt();
#endif
