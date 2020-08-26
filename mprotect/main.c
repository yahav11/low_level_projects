//#include <hook.h>
#include <manage_hooks.h>

char original_function_address_opcode[JMP_OPCODE_SIZE] = { 0 };
intptr_t caller_address = 0;


void hook_func_memory(
        void *original_func_ptr, 
        size_t start_offset, 
        u_int64_t *hook_func_address, 
        size_t opcode_size, 
        char *instruction)
{
	// calculate offset and page size
	size_t pagesize = sysconf(_SC_PAGE_SIZE);
	intptr_t start = (intptr_t)original_func_ptr,
             end = start + 1,
             page_start = start & -pagesize; 

    memcpy((instruction + ASIGN_OPCODE_OFFSET), &hook_func_address, X64ADDRESS_SIZE);

    // remove writing protection from this page
	if(mprotect((void *)page_start, 
                end - page_start, 
                PROT_READ | PROT_WRITE | PROT_EXEC) == -1)
	{
		printf("Error: %s\n", strerror(errno));
	}

    // copy the instruction set to the func address
	memcpy(((char *)original_func_ptr) + start_offset, instruction, opcode_size);
}


void hook(void *original_function_address, void *hooked_function)
{
    // save the opcode 
    memcpy(original_function_address_opcode, (char *)original_function_address, JMP_OPCODE_SIZE);

    // first bytes of good is jmp to bad
	hook_func_memory(original_function_address, 0, (u_int64_t *)hook_manager, JMP_OPCODE_SIZE, CALL_ADDRESS_OPCODE);
}

int main(void)
{
    hook(&good, &bad);
    insertFirst(&good, &bad);

    good(5, 6);


    printf("ACTUALLY ENDED\n\n");

	return 0;
}

u_int64_t get_return_address()
{
    asm("movq 8(%rbp), %rax;\n\t");
}

void last_fix()
{
    // This is padding for the stack
    char padding[22];  
    
    hook(&good, &bad);
    
    // Fix stack before jump to original rip
    asm("\t add $0x20, %rsp");
    asm("\t popq %rbp");

    // Jump to the caller
    asm("\t jmp *%0" : : "r" (caller_address)); 
}

void hook_manager()
{
    intptr_t original_function_address;

    // get return address and align it
    asm(" \t pushq %rdi");
    asm(" \t pushq %rsi");
    
    asm("\t call *%0" : : "r" (&bad));

    // original function return address
    asm("\t movq 8(%%rbp), %0" : "=r" (original_function_address));
    asm("\t movq 16(%%rbp), %0" : "=r" (caller_address));

    // Fix stackframe using putchar
    putchar(0);

    original_function_address -= JMP_OPCODE_SIZE;

    // calling the return address
    void (*func_ptr)(void) = (void (*)(void))original_function_address;

    memcpy((char *)func_ptr, original_function_address_opcode, JMP_OPCODE_SIZE);
    
    // change return address of the next function you are going to call
    
    // now the function should jump to bad
    // we need to inset to stuck the jumps to:
    // good, last_fix, main+x

    asm("\t movq %0, 8(%%rbp)" : : "r" (original_function_address));  
    asm("\t movq %0, 16(%%rbp)" : : "r" (&last_fix));  // intial address (rip before calling)
    
    // fix fast call
    asm(" \t popq %rsi");
    asm(" \t popq %rdi");
}

void bad(int x, int y)
{
    printf("bad = %d, %d\n", x, y);
}

void good(int x, int y)
{
    printf("good = %d, %d\n", x, y);
}
