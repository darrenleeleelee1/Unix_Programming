#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "got.h"
#include "function_list.h"
#include "config.h"
api_function_blacklist_t blacklist;
fd_info_t fd_read_info[MAX_FD];
fd_info_t fd_write_info[MAX_FD];
int logger_fd;
int debugger_fd = 1; // debug log to stdout
size_t num_fd = 0;
int __libc_start_main(int (*main) (int,char **,char **),
                      int argc, char **ubp_av,
                      void (*init) (void),
                      void (*fini)(void),
                      void (*rtld_fini)(void),
                      void (*stack_end)) {
    // debugger_fd = open("./debug.log", O_WRONLY | O_CREAT | O_TRUNC, 0644); // debug log to debug.log
    debugger_fd = open("/dev/null", O_WRONLY); // debug log to null

    char *api_function_list[] = {"open", "read", "write"
                    , "connect", "getaddrinfo", "system"};
    size_t num_function = sizeof(api_function_list) / sizeof(api_function_list[0]);
    loadConfigFile(api_function_list, num_function);
    char* logger_fd_str = getenv("LOGGER_FD");
    dprintf(debugger_fd, "LOGGER_FD: %s\n\n", logger_fd_str);
    if (logger_fd_str == NULL) {
        fprintf(stderr, "Error: LOGGER_FD environment variable not set\n");
        return 1;
    }
    logger_fd = atoi(logger_fd_str);

    unsigned long api_function_got_addr[6] = {0, 0, 0, 0, 0, 0};
    readRelocationEntries(api_function_list, num_function, api_function_got_addr);
    dprintf(debugger_fd, "%s %s\n", "Address", "Symbol");
    for(size_t i = 0; i < num_function; i++){
        dprintf(debugger_fd, "0x%lx %s\n", api_function_got_addr[i], api_function_list[i]);
    }
    
    unsigned long aligned_min_addr;
    unsigned long aligned_max_addr;
    alignedFunctionAddr(api_function_got_addr, num_function, &aligned_min_addr, &aligned_max_addr);
    size_t cross_pages = ((aligned_max_addr - aligned_min_addr) / sysconf(_SC_PAGESIZE)) + 2; // add 2 if some function cross pages
    
    dprintf(debugger_fd, "Aligned min address: 0x%lx\n", aligned_min_addr);
    dprintf(debugger_fd, "Aligned max address: 0x%lx\n", aligned_max_addr);
    dprintf(debugger_fd, "Cross %lu page(s)\n\n", cross_pages);

    FILE* fp;
    char buffer[1024];
    // Open the /proc/self/maps file
    fp = fopen("/proc/self/maps", "r");
    if (fp == NULL) {
        perror("Failed to open /proc/self/maps");
        return 1;
    }
    // Get the process start address
    char str_proc_addr[13];
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        strncpy(str_proc_addr, buffer, 12);
        str_proc_addr[12] = '\0';
        dprintf(debugger_fd, "Porc address: %s\n", str_proc_addr);
        break;
    }
    void *base_addr = (void *)strtol(str_proc_addr, NULL, 16); // convert the address string to a pointer
    void *got_addr_page = (void*)(unsigned long*)base_addr + aligned_min_addr;
    if (mprotect(got_addr_page, sysconf(_SC_PAGESIZE)*cross_pages, PROT_READ | PROT_WRITE) == -1) {
        perror("mprotect failed");
        return 1;
    }
    void *handle_sandbox_so = dlopen("sandbox.so", RTLD_LAZY);
    for(int i = 0; i < 6; i++){
        if(api_function_got_addr[i] == 0) continue;
        void *hijack_function_got = (void*) (unsigned long*)base_addr + api_function_got_addr[i];
        char hijack_function[20];
        sprintf(hijack_function, "hijack%s", api_function_list[i]);
        *(unsigned long*) hijack_function_got = (unsigned long)dlsym(handle_sandbox_so, hijack_function);
    }


    void *handle_libc_so_6 = dlopen("libc.so.6", RTLD_LAZY);
    typedef int (*libc_start_main_t)(int (*)(int, char **, char **), int, char **, void (*)(void), void (*)(void), void (*)(void), void *);
    libc_start_main_t real_libc_start_main = (libc_start_main_t)dlsym(handle_libc_so_6, "__libc_start_main");
    /* Call the real __libc_start_main function with the provided arguments */
    return real_libc_start_main(main, argc, ubp_av, init, fini, rtld_fini, stack_end);
    return 0;
}
