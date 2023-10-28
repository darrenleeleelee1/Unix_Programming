#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/user.h>
#include <capstone/capstone.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <elf.h>

#define MAX_INSTRUCTION_SIZE 6

struct Breakpoint {
    Elf64_Addr addr;
    unsigned char orig_code; // original code where breakpoint is set
};

struct SavedState {
    struct user_regs_struct regs;
    size_t text_section_size;
    char* text_section;
};

struct DebuggerState {
    pid_t pid;
    Elf64_Addr text_section_start;
    Elf64_Addr text_section_end;
    struct Breakpoint** breakpoints;
    size_t num_breakpoints;
    struct SavedState* saved_state;
};


// Returns the breakpoint at the specified address, or NULL if there is no breakpoint there.
struct Breakpoint* find_breakpoint(struct DebuggerState* state, Elf64_Addr addr) {
    for (size_t i = 0; i < state->num_breakpoints; i++) {
        if (state->breakpoints[i]->addr == addr) {
            return state->breakpoints[i];
        }
    }
    return NULL;
}

void set_breakpoint(struct DebuggerState* state, Elf64_Addr addr) {
    long data = ptrace(PTRACE_PEEKDATA, state->pid, addr, NULL);
    struct Breakpoint* bp = malloc(sizeof(struct Breakpoint));
    bp->addr = addr;
    bp->orig_code = data & 0xff; // preserve only the least significant byte
    long trap = (data & ~0xff) | 0xcc; // place the INT3 opcode in the least significant byte
    ptrace(PTRACE_POKEDATA, state->pid, addr, trap);
    state->breakpoints = realloc(state->breakpoints, sizeof(struct Breakpoint*) * (state->num_breakpoints + 1));
    state->breakpoints[state->num_breakpoints] = bp;
    state->num_breakpoints++;
}

void remove_breakpoint(struct DebuggerState* state, struct Breakpoint* bp) {
    long data = ptrace(PTRACE_PEEKTEXT, state->pid, bp->addr, NULL);
    long restored_data = (data & ~0xff) | bp->orig_code;
    ptrace(PTRACE_POKETEXT, state->pid, bp->addr, restored_data);
}

void save_state(struct DebuggerState* state) {
    state->saved_state = malloc(sizeof(struct SavedState));
    ptrace(PTRACE_GETREGS, state->pid, NULL, &state->saved_state->regs);
    state->saved_state->text_section_size = state->text_section_end - state->text_section_start;
    state->saved_state->text_section = malloc(state->saved_state->text_section_size);
    for (size_t i = 0; i < state->saved_state->text_section_size; i++) {
        state->saved_state->text_section[i] = ptrace(PTRACE_PEEKTEXT, state->pid, state->text_section_start + i, NULL);
    }
}

void restore_state(struct DebuggerState* state) {
    ptrace(PTRACE_SETREGS, state->pid, NULL, &state->saved_state->regs);

    for (size_t i = 0; i < state->saved_state->text_section_size; i++) {
        // ptrace(PTRACE_POKETEXT, state->pid, state->text_section_start + i, state->saved_state->text_section[i]);
    }

    free(state->saved_state->text_section);
    free(state->saved_state);
    state->saved_state = NULL;
}

void print_instructions(uint8_t* instruction, size_t size) {
    for (size_t i = 0; i < size; i++) {
        printf("%02x ", instruction[i]);
    }

    for (size_t i = size; i < MAX_INSTRUCTION_SIZE; i++) {
        printf("   ");
    }
}

void run_debugger(struct DebuggerState* state) {
    int wait_status, timetravel_status = 0, reset_breakpoint_status = 0;
    struct Breakpoint* reset_bp;
    struct user_regs_struct regs;
    csh handle;
    cs_insn *insn;
    size_t count;

    if (cs_open(CS_ARCH_X86, CS_MODE_64, &handle) != CS_ERR_OK) {
        printf("Failed to initialize capstone\n");
        return;
    }

    while (1) {
        if(timetravel_status == 1){
            printf("** go back to the anchor point\n");
            timetravel_status = 0;
        } else{
            waitpid(state->pid, &wait_status, 0);
        }
        if (WIFEXITED(wait_status)) {
            break;
        }
        ptrace(PTRACE_GETREGS, state->pid, NULL, &regs);

        size_t MAX_instruction_size = 50;
        uint8_t instruction[MAX_instruction_size];
        // If we've hit a breakpoint, restore the original instruction
        struct Breakpoint* bp = find_breakpoint(state, regs.rip -1);
        
        if (bp) {
            printf("** hit a breakpoint at 0x%llx\n", regs.rip - 1);
            remove_breakpoint(state, bp);
            regs.rip -= 1;  // Back up to the start of the original instruction
            ptrace(PTRACE_SETREGS, state->pid, NULL, &regs);
            // ptrace(PTRACE_SINGLESTEP, state->pid, NULL, NULL);
            // waitpid(state->pid, &wait_status, 0);
            // ... handle exit, signals, etc. ...
            reset_breakpoint_status = 1;
            reset_bp = bp;
            // set_breakpoint(state, bp->addr);  // Put breakpoint back in place here.
        }

        // Do not read beyond the text section
        for (int i = 0; i < MAX_instruction_size && regs.rip + i <= state->text_section_end; i += sizeof(long)) {
            long data = ptrace(PTRACE_PEEKTEXT, state->pid, regs.rip + i, NULL);
            // If this instruction overlaps with a breakpoint, restore the original instruction before printing.
            for(int j = i; j < i + sizeof(long); j++) {
                bp = find_breakpoint(state, regs.rip + j);
                if (bp) {
                    data = (data & ~(0xffULL << (8 * (j - i)))) | ((unsigned long long)(bp->orig_code) << (8 * (j - i)));
                }
            }
            memcpy(instruction + i, &data, sizeof(data));
        }

        size_t instruction_size = (regs.rip + MAX_instruction_size <= state->text_section_end) ? MAX_instruction_size : (state->text_section_end - regs.rip);
        count = cs_disasm(handle, instruction, instruction_size, regs.rip, 5, &insn);

        for(size_t i = 0; i < count; i++) {
            printf("      %lx: ", insn[i].address);
            print_instructions(insn[i].bytes, insn[i].size);
            printf("%s\t%s\n", insn[i].mnemonic, insn[i].op_str);
        }
        if (count < 5) printf("** the address is out of the range of the text section.\n");
        if (count > 0) cs_free(insn, count);
        else printf("Failed to disassemble given code!\n");

        // handle user commands, implement breakpoints, time travel, etc.
        char command[1024];
        printf("(sdb) ");
        while (fgets(command, sizeof(command), stdin)) {
            if (strcmp(command, "si\n") == 0) {
                ptrace(PTRACE_SINGLESTEP, state->pid, NULL, NULL);
                if(reset_breakpoint_status == 1) {
                    reset_breakpoint_status = 0;
                    set_breakpoint(state, reset_bp->addr);  // Put breakpoint back in place here.
                }
                break;
            } else if (strcmp(command, "cont\n") == 0) {
                if(reset_breakpoint_status == 1) {
                    reset_breakpoint_status = 0;
                    ptrace(PTRACE_SINGLESTEP, state->pid, NULL, NULL);
                    waitpid(state->pid, &wait_status, 0);
                    if (WIFEXITED(wait_status)) {
                        break;
                    }
                    set_breakpoint(state, reset_bp->addr);  // Put breakpoint back in place here.
                }
                ptrace(PTRACE_CONT, state->pid, NULL, NULL);
                break;
            } else if (strncmp(command, "break ", 6) == 0) {
                // Set breakpoint
                char *end;
                Elf64_Addr addr = strtoul(command + 6, &end, 16);
                set_breakpoint(state, addr);
                printf("** set a breakpoint at 0x%lx\n", addr);
            } else if (strcmp(command, "anchor\n") == 0) {
                // Set anchor
                save_state(state);
                printf("** dropped an anchor\n");
            } else if (strcmp(command, "timetravel\n") == 0) {
                // Time travel to last anchor
                if (state->saved_state) {
                    restore_state(state);
                }
                timetravel_status = 1;
                break;
            } else {
                printf("Invalid command: %s", command);
            }
            printf("(sdb) ");
        }
    }

    cs_close(&handle);
}

int main(int argc, char *argv[]) {
    struct DebuggerState state;
    state.num_breakpoints = 0;

    if (argc < 2) {
        printf("Please provide the ELF file name.\n");
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    Elf64_Ehdr ehdr;
    if (read(fd, &ehdr, sizeof(ehdr)) != sizeof(ehdr)) {
        perror("read");
        close(fd);
        return 1;
    }

    if (memcmp(ehdr.e_ident, ELFMAG, SELFMAG) == 0) { // check if it's an ELF file
        printf("** program '%s' loaded. entry point 0x%lx\n", argv[1], (unsigned long) ehdr.e_entry);

        lseek(fd, ehdr.e_phoff, SEEK_SET);
        for (int i = 0; i < ehdr.e_phnum; i++) {
            Elf64_Phdr phdr;
            read(fd, &phdr, sizeof(phdr));

            if (phdr.p_type == PT_LOAD && (phdr.p_flags & PF_X)) {
                state.text_section_start = phdr.p_vaddr;
                state.text_section_end = phdr.p_vaddr + phdr.p_filesz;
                break;
            }
        }

    } else {
        printf("This is not an ELF file.\n");
    }
    close(fd);

    pid_t pid = fork();
    if (pid == 0) {
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        execvp(argv[1], argv + 1);
    } else if (pid >= 1) {
        state.pid = pid;
        run_debugger(&state);
        printf("** the target program terminated.\n");
    }

    return 0;
}
