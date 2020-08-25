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

struct g_hooked_functions
{

};

u_int64_t *good_func = (u_int64_t *)&good;
char good_opcode[JMP_OPCODE_SIZE] = { 0 };
intptr_t caller_address = 0;

size_t get_fucntion_offset_to_return_address(void *function_ptr)
{
    int byte_counter = 0;

    while(((char *)function_ptr)[byte_counter] != RETURN_OPCODE)
    {
        byte_counter++;
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


void hook(void *original_function, void *hooked_function)
{
    //asm("\t movq 8(%%rsp), %0" : "=r" (main_address));
    //printf("main_address = %p\n", main_address);
    //asm("\t jmp %0" : : "r" (main_address));  

    size_t bad_ret_offset = get_fucntion_offset_to_return_address(hooked_function),
           //fix_ret_offset = get_fucntion_offset_to_return_address(&fix_hooked),
           good_ret_offset = get_fucntion_offset_to_return_address(original_function),
           hook_manager_ret_offset = get_fucntion_offset_to_return_address(hook_manager);

    char jmp_rax_address_opcode[JMP_OPCODE_SIZE] = \
         {0x48, 0xb8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xe0}; 

    char call_address_opcode[CALL_OPCODE_SIZE] = \
         {0x48, 0xb8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xd0};      
    
    u_int64_t *u_hook_manager = (u_int64_t *)hook_manager;//&bad; 
    u_int64_t *bad_func = (u_int64_t *)hooked_function;//&bad; 
    //u_int64_t *u_fix_hooked = (u_int64_t *)&fix_hooked;

    // save the opcode 
    memcpy(good_opcode, (char *)original_function, JMP_OPCODE_SIZE);

    // first bytes of good is jmp to bad
	hook_func_memory(original_function, 0, u_hook_manager, JMP_OPCODE_SIZE, call_address_opcode);
    
    // last bytes of bad is jmp to fix_hooked
    //hook_func_memory(hook_manager, hook_manager_ret_offset - 8, bad_func, JMP_OPCODE_SIZE, jmp_rax_address_opcode);
    /*
    // last bytes of fix_hooked is jmp to good
    hook_func_memory(&fix_hooked, fix_ret_offset, good_func, JMP_OPCODE_SIZE, jmp_rax_address_opcode);
    */
}

int main(void)
{
    int i = 0;
    //printf("MAIN: %p\n", &main);
    bad(3);


    hook(&good, &bad);

    good(5);

    /*for (i = 0 ; i < 5 ; i++)
    {
        printf("BLIP %d\n", i);
        good(4);
        printf("BLOOP %d\n", i);
    }
    */
    printf("ACTUALLY ENDED\n\n");


	return 0;
}

void good(int x)
{
	printf("good = %d\n", x);

    printf("Where is good returning to?\n");
    return;
}

u_int64_t get_return_address()
{
    asm("movq 8(%rbp), %rax;\n\t");
}

/*void change_return_address()
{
    asm("\t movq %0, 8(%%rbp)" : : "r" (&second_fix_hook));
}*/
/*
void fix_hook(void *original_function)
{
    intptr_t return_address;
    
    asm("\t movq 8(%%rbp), %0" : "=r" (return_address));  // get the intial address (rip before calling)
    //asm("\t movq %0, 8(%%rbp)" : : "r" (original_function));  // TODO: not permanent
    asm("\t movq %0, 8(%%rbp)" : : "r" (return_address));  // intial address (rip before calling)
    asm("\t movq %0, 16(%%rbp)" : : "r" (&last_fix));  // push last_fix function
    printf("fix_hook\n");
}*/

void last_fix()
{
    printf("last_fix\n");
    intptr_t return_address;
    asm("\t movq 8(%%rbp), %0" : "=r" (return_address));
    //printf("original_address = %p\n", return_address);
    //printf("good = %p\n", &good);

    char call_address_opcode[CALL_OPCODE_SIZE] = \
         {0x48, 0xb8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xd0};       

    // hook original function all over again
    u_int64_t *u_hook_manager = (u_int64_t *)hook_manager;  //&bad;  
    //hook_func_memory(&good, 0, u_hook_manager, JMP_OPCODE_SIZE, call_address_opcode);
    hook(&good, &bad);
    printf("done?\n");

    printf("before epi\n");
    // Our own epilogue to the function
    //asm("\t int3"); // 0xCC

    //asm("\t leave");
    // same as
    asm("\t add $0x20, %rsp");
    asm("\t popq %rbp");

    printf("after epi\n");

    // Jump to the caller
    asm("\t jmp *%0" : : "r" (caller_address)); 
}

void hook_manager()
{
    intptr_t rsp;
    // get return address and align it
    asm("\t mov %%rbp, %0" : "=rm" (rsp));


    printf("before = %p\n", rsp);
    asm("\t call *%0" : : "r" (&bad));
    //asm("\t sub $0x10, %rbp");

    asm("\t mov %%rbp, %0" : "=rm" (rsp));
    printf("after = %p\n", rsp);

    asm("\t call *%0" : : "r" (&bad));

    // original function return address
    intptr_t original_function;
    asm("\t movq 8(%%rbp), %0" : "=r" (original_function));

    asm("\t movq 16(%%rbp), %0" : "=r" (caller_address));
    printf("CALLER TO GOOD IS ACTUALLY %p\n", caller_address);

    original_function -= JMP_OPCODE_SIZE;

    // calling the return address
    void (*func_ptr)(void) = (void (*)(void))original_function;

    memcpy((char *)func_ptr, good_opcode, JMP_OPCODE_SIZE);
    
    // change return address of the next function you are going to call
    //asm("\t movq %0, 8(%%rbp)" : : "r" (&second_fix_hook));
    printf("hook_manager\n");
    
    // now the function should jump to bad
    // we need to inset to stuck the jumps to:
    // good, last_fix, main+x


    //asm("\t movq %0, 8(%%rbp)" : : "r" (&bad));  // TODO get from struct
    asm("\t movq %0, 8(%%rbp)" : : "r" (original_function));  
    asm("\t movq %0, 16(%%rbp)" : : "r" (&last_fix));  // intial address (rip before calling)

    //asm("\t movq %0, 32(%%rbp)" : : "r" (original_function)); 
    //asm("\t movq %0, 40(%%rbp)" : : "r" (&main));  // TODO get caller address
    //asm(NOP_SLED);
    
    printf("Returning from hook_manager, where to?\n");
    return;
}

void bad(int *x)
{
    printf("bad = %d\n", x);
}
