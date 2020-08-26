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


struct node {
   //int data;  // delete later
   //int key;   // delete later

   intptr_t *original_function;
   intptr_t *hooked_function;

   char original_function_address_opcode[JMP_OPCODE_SIZE];
   intptr_t caller_address;

   struct node *next;
};

struct node *head = NULL;
struct node *current = NULL;


intptr_t ontime_original_function;
intptr_t ontime_hooked_function;
intptr_t ontime_caller_address;

void hook_func_memory();
u_int64_t get_return_address();
void good(int x, int y);
void bad(int x, int y);
void fix_hook();
void hook_manager();
void last_fix();

void hook(void *original_function_address, void *hooked_function, struct node* current)
{
    // save the opcode 
    memcpy(current->original_function_address_opcode, (char *)original_function_address, JMP_OPCODE_SIZE);

    // first bytes of good is jmp to bad
	hook_func_memory(original_function_address, 0, (u_int64_t *)hook_manager, JMP_OPCODE_SIZE, CALL_ADDRESS_OPCODE);
}