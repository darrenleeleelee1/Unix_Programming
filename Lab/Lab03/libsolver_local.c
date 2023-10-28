#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdint.h>
#include <dlfcn.h>
#include "libsolver_local.h"
#include "shuffle.h"
void modifyGOT(char *function_num, void *got_address){
    void *handle = dlopen("./libpoem.so", RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "Error: %s\n", dlerror());
        exit(EXIT_FAILURE);
    }
    
    char function_name[11] = "";
    strcat(function_name, "code_");
    strcat(function_name, function_num);
    printf("%s: %p\n", function_name, got_address);

    *(unsigned long*)got_address = (unsigned long)dlsym(handle, function_name);
    
    char *error = dlerror();
    if (error != NULL) {
        fprintf(stderr, "Error: %s\n", error);
        exit(EXIT_FAILURE);
    }
    int result = dlclose(handle);
    if (result != 0) {
        fprintf(stderr, "Error: %s\n", dlerror());
        exit(EXIT_FAILURE);
    }
}
int init(){
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
        printf("Porc address: %s\n", str_proc_addr);
        break;
    }
    void *base_addr = (void *) strtol(str_proc_addr, NULL, 16); // convert the address string to a pointer
    void *got_addr_page = (void*) (unsigned long*)base_addr+0x24000;
    if (mprotect(got_addr_page, sysconf(_SC_PAGESIZE)*2, PROT_READ | PROT_WRITE) == -1) {
        perror("mprotect failed");
        return 1;
    }

    for(size_t i = 0; i < sizeof(GOT_offset) / sizeof(GOT_offset[0]); i++){
        if(GOT_offset[i] == 0) continue;
        void *got_address = (void*) (unsigned long*)base_addr + GOT_offset[i];
        char function_num[6];
        int real_id = 0;
        for(size_t j = 0; j < sizeof(ndat) / sizeof(ndat[0]); j++){
            if(ndat[j] == i){
                real_id = j;
                break;
            }
        }
        snprintf(function_num, sizeof(function_num),"%d", real_id);
        modifyGOT(function_num, got_address);
    }

    // Close the file
    fclose(fp);

    return 0;
}

