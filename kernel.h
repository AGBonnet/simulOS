#ifndef KERNEL_H
#define KERNEL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shell.h"
#include "cpu.h"
#include "pcb.h"
#include "memorymanager.h"
#include "DISK_driver.h"

char* ram[40];
struct CPU cpu;
struct readyQ* ready;
int main();
void boot();
int kernel();
//int myinit (char *filename);
void addToReady (struct PCB *newPCB);
int scheduler();
#endif         
