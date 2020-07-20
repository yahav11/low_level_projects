#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>


#define OPCODE_SIZE (16)
#define X64ADDRESS_SIZE (8)
#define ASIGN_OPCODE_OFFSET (2)
#define RETURN_OPCODE (-61)
#define BYTE_SIZE (8)


void hook_func_memory();
void good();
void bad();
void fix_hooked();
void second_fix_hook();
