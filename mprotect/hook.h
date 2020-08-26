#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>

#define RETURN_OPCODE (-61)
#define JMP_OPCODE_SIZE (12)
#define CALL_OPCODE_SIZE (12)
#define STDCALL __attribute__((stdcall))

const int X64ADDRESS_SIZE = 8;
const int BYTE_SIZE = 8;
const int ASIGN_OPCODE_OFFSET = 2;
char CALL_ADDRESS_OPCODE[CALL_OPCODE_SIZE] = \
         {0x48, 0xb8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xd0};     

void hook_func_memory();
u_int64_t get_return_address();
void good(int x, int y);
void bad(int x, int y);
void fix_hook();
void hook_manager();
void last_fix();
