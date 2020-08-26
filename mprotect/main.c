#include <manage_hooks.h>


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


int main(void)
{
    printf("main is : %p\n", &main);
    push_hook(&good, &bad);

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
    struct node* current_node;
    intptr_t original_caller;


    hook(&good, &bad, current_node);
    
    // Fix stack before jump to original rip
    asm("\t add $0x20, %rsp");
    asm("\t popq %rbp");
    
    //ontime_original_function = original_function_address;
    //ontime_hooked_function = current_node->hooked_function;
    //ontime_caller_address

    // Jump to the caller
    asm("\t jmp *%0" : : "r" (ontime_caller_address)); 
}

void hook_manager()
{
    intptr_t original_function_address;
    struct node* current_node;

    // Save fastcall opcodes
    asm(" \t pushq %rdi");
    asm(" \t pushq %rsi");

    // original function return address
    asm("\t movq 8(%%rbp), %0" : "=r" (original_function_address));

    // Find the current node that contains all the information about the jook
    current_node = find_node((void *)(original_function_address - JMP_OPCODE_SIZE));

    // Restore fastcall opcodes and save them again
    asm(" \t popq %rsi");
    asm(" \t popq %rdi");
    asm(" \t pushq %rdi");
    asm(" \t pushq %rsi");

    asm("\t call *%0" : : "r" (current_node->hooked_function));
    asm("\t movq 16(%%rbp), %0" : "=r" (current_node->caller_address));
    
    // Fix stackframe using putchar
    putchar(0);

    original_function_address -= JMP_OPCODE_SIZE;


    // calling the return address
    void (*func_ptr)(void) = (void (*)(void))original_function_address;

    memcpy((char *)func_ptr, current_node->original_function_address_opcode, JMP_OPCODE_SIZE);
    
    // change return address of the next function you are going to call
    
    // now the function should jump to bad
    // we need to inset to stuck the jumps to:
    // good, last_fix, main+x

    asm("\t movq %0, 8(%%rbp)" : : "r" (original_function_address));  
    asm("\t movq %0, 16(%%rbp)" : : "r" (&last_fix));  // intial address (rip before calling)


    ontime_original_function = (intptr_t)original_function_address;
    ontime_hooked_function = (intptr_t)current_node->hooked_function;
    ontime_caller_address = (intptr_t)current_node->caller_address;

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
