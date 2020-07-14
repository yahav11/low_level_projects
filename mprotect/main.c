#include <hook.h>


void hook_func_memory(void *original_func_ptr, void *hook_func_ptr)
{
	// functions pointers
	u_int64_t *original_func = (u_int64_t *)original_func_ptr; //&good;
	u_int64_t *hook_func = (u_int64_t *)hook_func_ptr; //&bad;

	// calculate offset and page size
	size_t pagesize = sysconf(_SC_PAGE_SIZE);
	intptr_t start = (intptr_t)original_func,
             end = start + 1,
             page_start = start & -pagesize; 
	char instruction[OPCODE_SIZE] = \
        {0x48, 0xb8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xe0};
    


    // Copy the function memory address (x64) to the instruction set
    memcpy((instruction + ASIGN_OPCODE_OFFSET), &hook_func, X64ADDRESS_SIZE);



    // remove writing protection from this page
	if(mprotect((void *)page_start, 
                end - page_start, 
                PROT_READ | PROT_WRITE | PROT_EXEC) == -1)
	{
		printf("Error: %s\n", strerror(errno));
	}

    // copy the instruction set to the func address
	memcpy(original_func, instruction, OPCODE_SIZE);

    // remove write privillige from the function page
	if(mprotect((void *)page_start, end - page_start, PROT_READ | PROT_EXEC) == -1)
		printf("Error: %s\n", strerror(errno));
}

int main()
{
	// remove 12 bits in order to align the page value
	hook_func_memory(&good, &bad);

    // call the hooked function
    good();

	return 0;
}

void good()
{
	printf("h3llo world!\n");
}

void bad()
{
	printf("Goodbye cruel world!\n");
}
