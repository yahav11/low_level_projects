#include <hook.h>

// Globals
u_int64_t *bad_func = (u_int64_t *)&bad; //&bad;
u_int64_t *good_func = (u_int64_t *)&good; //&bad;
u_int64_t *fix_func = (u_int64_t *)&fix_hooked; //&bad;
u_int64_t *second_fix = (u_int64_t *)&second_fix_hook; 
char good_opcode[12] = { 0 };


size_t get_fucntion_offset_to_return_address(void *function_ptr)
{
    int byte_counter = 0;

    while(((char *)function_ptr)[byte_counter] != RETURN_OPCODE)
    {
        byte_counter++;
    }

    return byte_counter;
}


void write_return_address_to_instruction_set(void *function_ptr, char *instruction_set)
{
    size_t bytes_offset = get_fucntion_offset_to_return_address(&function_ptr);
    memcpy(instruction_set, function_ptr + bytes_offset, 8);
}


void hook_func_memory(
        void *original_func_ptr, 
        void *hook_func_ptr, 
        size_t start_offset, 
        u_int64_t *hook_func_address, 
        size_t opcode_size, 
        char *instruction)
{
	// functions pointers
	u_int64_t *original_func = (u_int64_t *)original_func_ptr; //&good;
	//u_int64_t *hook_func = (u_int64_t *)hook_func_ptr; //&bad;


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


void copy_hooked_function_instruction(void *function_ptr, char *instruction, size_t size_to_read)
{
	u_int64_t *hooked_function = (u_int64_t *)&function_ptr; //&bad;
    memcpy(good_opcode, &hooked_function, 12);
}

void fix_hooked() 
{
	memcpy(good_func, good_opcode, 12);

    __asm("NOP");
    __asm("NOP");
    __asm("NOP");
    __asm("NOP");
    __asm("NOP");
    __asm("NOP");

    //asm("push %0" : "=r" (second_fix));
}

void second_fix_hook()
{
    printf("\nHello Im here!!!");
}

void hook()
{

    size_t bad_ret_offset = get_fucntion_offset_to_return_address(&bad),
           fix_ret_offset = get_fucntion_offset_to_return_address(&fix_hooked),
           good_ret_offset = get_fucntion_offset_to_return_address(&good);
    char jmp_rax_address_opcode[12] = \
         {0x48, 0xb8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
             0x00, 0x00, 0xff, 0xe0}; 


    memcpy(good_opcode, (char *)&good, 12);

    // first bytes of good is jmp to bad
	hook_func_memory(&good, &bad, 0, bad_func, 12, jmp_rax_address_opcode);
    
    // last bytes of bad is jmp to fix_hooked
    hook_func_memory(&bad, &fix_hooked, bad_ret_offset, fix_func, 12, jmp_rax_address_opcode);

    // last bytes of fix_hooked is jmp to good
    hook_func_memory(&fix_hooked, &good, fix_ret_offset, good_func, 12, jmp_rax_address_opcode);
   
    //asm("push %0" : "=r" (second_fix));

    // Hadded this!!
    // last bytes of good is jmp to second_fix_hook
    /*
    hook_func_memory(&good, &second_fix_hook, good_ret_offset, second_fix, 12, jmp_rax_address_opcode);
    */
    //asm("push ")
}

int main()
{
    hook();
    good();

	return 0;
}

void good()
{
	printf("h3llo world!\n");
}

void bad()
{
    int i = 0;
    int x = 0;

    for(i = 0; i < 15; i++)
    {
        x += i;
    }

    printf("x = %d\n", x);
    
	printf("Goodbye cruel world!\n");
}
