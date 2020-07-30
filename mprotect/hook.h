#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#define RETURN_OPCODE (-61)
#define JMP_OPCODE_SIZE (12)
#define CALL_OPCODE_SIZE (12)
// 12 nopes
#define NOP_SLED "nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;nop;"

const int X64ADDRESS_SIZE = 8;
const int BYTE_SIZE = 8;
const int ASIGN_OPCODE_OFFSET = 2;

void hook_func_memory();
void good(int x);
void bad();
void fix_hooked();
void second_fix_hook();
void hook_manager();
