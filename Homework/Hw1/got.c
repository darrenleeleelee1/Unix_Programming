#include <stdio.h>
#include <elf.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/mman.h>
#include "got.h"
#include "config.h"
int readRelocationEntries(char *api_function_list[],  size_t api_function_list_size, unsigned long int result[]) {
    char *elf_path = "/proc/self/exe";
    // Open the binary file
    int fd = open(elf_path, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    // Read ELF header
    Elf64_Ehdr ehdr;
    if (read(fd, &ehdr, sizeof(ehdr)) != sizeof(ehdr)) {
        perror("read");
        close(fd);
        return 1;
    }

    // Validate the ELF header
    if (memcmp(ehdr.e_ident, ELFMAG, SELFMAG) != 0 || ehdr.e_ident[EI_CLASS] != ELFCLASS64) {
        fprintf(stderr, "Not a valid ELF64 file\n");
        close(fd);
        return 1;
    }

    // Read the section headers
    Elf64_Shdr shdrs[ehdr.e_shnum];
    if (lseek(fd, ehdr.e_shoff, SEEK_SET) == (off_t)-1 ||
        read(fd, shdrs, ehdr.e_shnum * sizeof(Elf64_Shdr)) != ehdr.e_shnum * sizeof(Elf64_Shdr)) {
        perror("read");
        close(fd);
        return 1;
    }

    // Read section header string table
    char *shstrtab = malloc(shdrs[ehdr.e_shstrndx].sh_size);
    if (shstrtab == NULL ||
        lseek(fd, shdrs[ehdr.e_shstrndx].sh_offset, SEEK_SET) == (off_t)-1 ||
        read(fd, shstrtab, shdrs[ehdr.e_shstrndx].sh_size) != shdrs[ehdr.e_shstrndx].sh_size) {
        perror("read");
        free(shstrtab);
        close(fd);
        return 1;
    }

    Elf64_Shdr *rela_plt_shdr = NULL;
    Elf64_Shdr *dynsym_shdr = NULL;
    Elf64_Shdr *dynstr_shdr = NULL;

    for (int i = 0; i < ehdr.e_shnum; i++) {
        const char *name = shstrtab + shdrs[i].sh_name;
        if (strcmp(name, ".rela.plt") == 0) {
            rela_plt_shdr = &shdrs[i];
        } else if (strcmp(name, ".dynsym") == 0) {
            dynsym_shdr = &shdrs[i];
        } else if (strcmp(name, ".dynstr") == 0) {
            dynstr_shdr = &shdrs[i];
        }
    }

    if (rela_plt_shdr != NULL && dynsym_shdr != NULL && dynstr_shdr != NULL) {
        dprintf(debugger_fd, "Found .rela.plt, .dynsym, and .dynstr sections\n");

        // Read the .dynsym section
        Elf64_Sym *dynsym = malloc(dynsym_shdr->sh_size);
        if (dynsym == NULL ||
            lseek(fd, dynsym_shdr->sh_offset, SEEK_SET) == (off_t)-1 ||
            read(fd, dynsym, dynsym_shdr->sh_size) != dynsym_shdr->sh_size) {
            perror("read");
            free(dynsym);
            free(shstrtab);
            close(fd);
            return 1;
        }

        // Read the .dynstr section
        char *dynstr = malloc(dynstr_shdr->sh_size);
        if (dynstr == NULL ||
            lseek(fd, dynstr_shdr->sh_offset, SEEK_SET) == (off_t)-1 ||
            read(fd, dynstr, dynstr_shdr->sh_size) != dynstr_shdr->sh_size) {
            perror("read");
            free(dynstr);
            free(dynsym);
            free(shstrtab);
            close(fd);
            return 1;
        }

        // Read the .rela.plt section
        Elf64_Rela *rela_plt = malloc(rela_plt_shdr->sh_size);
        if (rela_plt == NULL ||
            lseek(fd, rela_plt_shdr->sh_offset, SEEK_SET) == (off_t)-1 ||
            read(fd, rela_plt, rela_plt_shdr->sh_size) != rela_plt_shdr->sh_size) {
            perror("read");
            free(rela_plt);
            free(dynstr);
            free(dynsym);
            free(shstrtab);
            close(fd);
            return 1;
        }

        dprintf(debugger_fd, "Relocation entries of the six function:\n");
        for (size_t i = 0; i < rela_plt_shdr->sh_size / sizeof(Elf64_Rela); i++) {
            Elf64_Rela *rela = &rela_plt[i];
            Elf64_Xword info = rela->r_info;
            Elf64_Word sym = ELF64_R_SYM(info);

            Elf64_Sym *symbol = &dynsym[sym];
            const char *sym_name = dynstr + symbol->st_name;
            dprintf(debugger_fd, "%s # %d # %s\n", dynstr, symbol->st_name, sym_name);
            for(size_t i = 0; i < api_function_list_size; i++){
                if(strcmp(sym_name, api_function_list[i]) == 0){
                    dprintf(debugger_fd, "%lx %s\n", rela->r_offset, sym_name);
                    result[i] = rela->r_offset;
                }
            }
            
        }

        // Free resources and close the file
        free(rela_plt);
        free(dynstr);
        free(dynsym);
    } else {
        dprintf(debugger_fd, "Could not find .rela.plt, .dynsym, or .dynstr sections\n");
    }

    // Clean up
    free(shstrtab);
    close(fd);

    return 0;
}
void alignedFunctionAddr(unsigned long api_function_got_addr[], size_t num_function
                        , unsigned long *aligned_min_addr, unsigned long *aligned_max_addr){
    
    unsigned long int min_addr = ULONG_MAX;
    unsigned long int max_addr = 0;

    for (size_t i = 0; i < num_function; i++) {
        if (api_function_got_addr[i] != 0) {
            if (api_function_got_addr[i] < min_addr) {
                min_addr = api_function_got_addr[i];
            }
            if (api_function_got_addr[i] > max_addr) {
                max_addr = api_function_got_addr[i];
            }
        }
    }

    *aligned_min_addr = min_addr & ~(sysconf(_SC_PAGESIZE) - 1);
    *aligned_max_addr = max_addr & ~(sysconf(_SC_PAGESIZE) - 1);

    dprintf(debugger_fd, "Min address: 0x%lx\n", min_addr);
    dprintf(debugger_fd, "Max address: 0x%lx\n\n", max_addr);
}
