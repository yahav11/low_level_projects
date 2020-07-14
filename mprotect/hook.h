#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>


#define OPCODE_SIZE (12)
#define X64ADDRESS_SIZE (8)
#define ASIGN_OPCODE_OFFSET (2)


void hook_func_memory();
void good();
void bad();