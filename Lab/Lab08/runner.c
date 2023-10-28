#include <stdio.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <unistd.h>
#define MAGIC_LENGTH 10 // 9 digits + null terminator
// #define debugger_fd 1 // debug log to stdout
#define debugger_fd 2 // debug log to error

void toBinary(char *buffer, unsigned long value, int buffer_size) {
    for (int i = buffer_size - 2; i >= 0; i--, value >>= 1) {
        buffer[i] = (value & 1) ? '1' : '0';
    }
    buffer[buffer_size - 1] = '\0'; // Null-terminate the string
}


int main(int argc, char *argv[]) {
    pid_t pid;
    int status;
    size_t cc_cnt = 0;
    struct user_regs_struct regs;
    struct user_regs_struct regs_for_restore;
    unsigned long magic_address;
    unsigned long magic_value = 0; // Start with "000000000"
    char magic[MAGIC_LENGTH] = {0};


    if (argc < 2) {
        fprintf(stderr, "Usage: %s <prog>\n", argv[0]);
        return 1;
    }

    pid = fork();
    if (pid == -1) {
        perror("fork");
        return 1;
    }

    if (pid == 0) { // Child process
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        execv(argv[1], argv + 1);
        perror("execv");
        return 1;
    }
    /* Continue from start point */
    if (waitpid(pid, &status, 0) < 0) {
        perror("waitpid");
        return 1;
    }
    // Check if the child process has received a SIGTRAP signal
    if (WIFSTOPPED(status) && WSTOPSIG(status) == SIGTRAP) {
        // dprintf(debugger_fd, "%lu's CC\n", ++cc_cnt);
    }
    // Continue the child process
    if (ptrace(PTRACE_CONT, pid, NULL, 0) < 0) {
        perror("ptrace(CONT)");
        return 1;
    }

    while (1) { // Parent process
        if (waitpid(pid, &status, 0) < 0) {
            perror("waitpid");
            return 1;
        }

        // Check if the child process has received a SIGTRAP signal
        if (WIFSTOPPED(status) && WSTOPSIG(status) == SIGTRAP) {
            dprintf(debugger_fd, "%lu's CC\n", ++cc_cnt);

            // Get the current register values
            if (ptrace(PTRACE_GETREGS, pid, NULL, &regs) < 0) {
                perror("ptrace(GETREGS)");
                return 1;
            }
            switch (cc_cnt)
            {
            case 2:
                // memset(magic, '0', sizeof(magic)-1);
                dprintf(debugger_fd, "Store the magic address from RAX.\n");
                dprintf(debugger_fd, "RAX: %llx\n", regs.rax);
                magic_address = regs.rax;
                break;
            case 3:
                // if(oracle_connect() < 0) errquit("connect");
                dprintf(debugger_fd, "Store all the regs for restore.\n");
                regs_for_restore = regs;
                break;
            case 5:
                // end else;
                dprintf(debugger_fd, "RAX: %llx\n", regs.rax);
                if(regs.rax == 0xffffffff){
                    regs = regs_for_restore;
                    cc_cnt = 3;
                    if (ptrace(PTRACE_SETREGS, pid, NULL, &regs) < 0) {
                        perror("ptrace(SETREGS)");
                        return 1;
                    }

                    // Increment the magic value and write it to the child process's memory
                    magic_value++;
                    toBinary(magic, magic_value, MAGIC_LENGTH);
                    for (int i = 0; i < MAGIC_LENGTH; i += sizeof(long)) {
                        if (ptrace(PTRACE_POKEDATA, pid, magic_address + i,
                            *(unsigned long *)(magic + i)) < 0) {
                            perror("ptrace(POKEDATA)");
                            return 1;
                        }
                    }
                }
                
                break;
            default:
                break;
            }

            // Continue the child process
            if (ptrace(PTRACE_CONT, pid, NULL, 0) < 0) {
                perror("ptrace(CONT)");
                return 1;
            }
        }

        // If the child process has exited, break out of the loop
        if (WIFEXITED(status)) {
            dprintf(debugger_fd, "Child process exited with status %d\n", WEXITSTATUS(status));
            break;
        }
    }

    return 0;
}
