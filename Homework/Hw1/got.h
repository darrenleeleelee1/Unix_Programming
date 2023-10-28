#include <stdlib.h>
int readRelocationEntries(char *api_function_list[],  size_t api_function_list_size, unsigned long int result[]);
void alignedFunctionAddr(unsigned long api_function_got_addr[], size_t num_function
                        , unsigned long *aligned_min_addr, unsigned long *aligned_max_addr);