#include <hook.h>

/*
struct g_function_hook_dict
{
    char original_function_start_opcode[JMP_OPCODE_SIZE];
    u_int64_t *original_address;
    u_int64_t *hooked_address;
    struct g_function_hook_dict *next;
} typedef function_hook_dict; 
*/

u_int64_t *good_func = (u_int64_t *)&good;
char good_opcode[JMP_OPCODE_SIZE] = { 0 };


size_t get_fucntion_offset_to_return_address(void *function_ptr)
{
    int byte_counter = 0;

    while(((char *)function_ptr)[byte_counter] != RETURN_OPCODE)
    {
        byte_counter++;
    }

    return byte_counter;
}

size_t get_negative_offset_to_the_function_start_address(void *function_ptr)
{
     int byte_counter = 0;

    while(((char *)function_ptr)[byte_counter] != 0x48)// &&
        //((char *)function_ptr)[byte_counter - 1] != 0xb8)
    {
        printf("%hhX, ", ((char *)function_ptr)[byte_counter]);
        byte_counter--;
    }

    return byte_counter;
}

void hook_func_memory(
        void *original_func_ptr, 
        size_t start_offset, 
        u_int64_t *hook_func_address, 
        size_t opcode_size, 
        char *instruction)
{
	// functions pointers
	u_int64_t *original_func = (u_int64_t *)original_func_ptr; 

	// calculate offset and page size
	size_t pagesize = sysconf(_SC_PAGE_SIZE);
	intptr_t start = (intptr_t)original_func,
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
	memcpy(((char *)original_func) + start_offset, instruction, opcode_size);

    // remove write privillige from the function page - return it later
    /*
	if(mprotect((void *)page_start, end - page_start, PROT_READ | PROT_EXEC) == -1)
		printf("Error: %s\n", strerror(errno));
    */
}


void fix_hooked() 
{
    printf("Hi to you all\n");
	memcpy(good_func, good_opcode, JMP_OPCODE_SIZE);
}

void hook(void *original_function, void *hooked_function)
{
    size_t bad_ret_offset = get_fucntion_offset_to_return_address(hooked_function),
           fix_ret_offset = get_fucntion_offset_to_return_address(&fix_hooked),
           good_ret_offset = get_fucntion_offset_to_return_address(original_function),
           hook_manager_ret_offset = get_fucntion_offset_to_return_address(hook_manager);

    char jmp_rax_address_opcode[JMP_OPCODE_SIZE] = \
         {0x48, 0xb8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xe0}; 

    char call_address_opcode[CALL_OPCODE_SIZE] = \
         {0x48, 0xb8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xd0};      
    
    u_int64_t *u_hook_manager = (u_int64_t *)hook_manager;//&bad; 
    u_int64_t *bad_func = (u_int64_t *)hooked_function;//&bad; 
    u_int64_t *u_fix_hooked = (u_int64_t *)&fix_hooked;

    // save the opcode 
    memcpy(good_opcode, (char *)original_function, JMP_OPCODE_SIZE);

    // first bytes of good is jmp to bad
	hook_func_memory(original_function, 0, u_hook_manager, JMP_OPCODE_SIZE, call_address_opcode);
    
    // last bytes of bad is jmp to fix_hooked
    hook_func_memory(hook_manager, hook_manager_ret_offset - 8, bad_func, JMP_OPCODE_SIZE, jmp_rax_address_opcode);
    /*
    // last bytes of fix_hooked is jmp to good
    hook_func_memory(&fix_hooked, fix_ret_offset, good_func, JMP_OPCODE_SIZE, jmp_rax_address_opcode);
    */
}

int main()
{
    hook(&good, &bad);
    good(4);

	return 0;
}

void good(int x)
{

    printf("\nthe value is %d\n", x);
	printf("h3llo world!\n");
}

u_int64_t get_return_address()
{
    asm("movq 8(%rbp), %rax;\n\t");
}

void hook_manager()
{
    // get return address and align it
    intptr_t return_address;
    asm("\t movq 8(%%rbp), %0" : "=r" (return_address));
    return_address -= JMP_OPCODE_SIZE;

    // calling the return address
    void (*func_ptr)(void) = (void (*)(void))return_address;

    memcpy((char *)func_ptr, good_opcode, JMP_OPCODE_SIZE);
    func_ptr();

    // NOP sled in order to inset the commands
    asm(NOP_SLED);
}

void bad()
{
    //void (*func_ptr)(void) = (void (*)(void))return_address;
    //int neg_ret_offset = 0;
    printf("Im fucking badddd\n");
}
