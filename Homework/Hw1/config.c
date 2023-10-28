#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include "config.h"
void loadConfigFile(char *api_function_list[], size_t api_function_list_size){
    memset(&blacklist, 0, sizeof(blacklist));
    const char* filename = getenv("SANDBOX_CONFIG");
    FILE *fp;
    char line[1024];
    char *BEGIN = "BEGIN", *END = "END";
    size_t len_BEGIN = sizeof(*BEGIN) / sizeof(char);
    size_t len_END = sizeof(*END) / sizeof(char);
    fp = fopen(filename, "r");
    // initial blacklist
    for(size_t i = 0; i < api_function_list_size; i++){
        switch (i)
        {
        case 0:
            blacklist.open.count = 0;
            break;
        case 1:
            blacklist.read.count = 0;
            break;
        case 2:
            blacklist.write.count = 0;
            break;
        case 3:
            blacklist.connect.count = 0;
            break;
        case 4:
            blacklist.getaddrinfo.count = 0;
            break;
        case 5:
            blacklist.system.count = 0;
            break;
        default:
            break;
        }
    }
    while (fgets(line, sizeof(line), fp)) {
        if(strncmp(line, BEGIN, len_BEGIN) == 0){
            for(size_t i = 0; i < api_function_list_size; i++){
                size_t len_func = strlen(api_function_list[i]);
                if(strncmp(line + 6, api_function_list[i], len_func) == 0){
                    while (fgets(line, sizeof(line), fp)){
                        if(strncmp(line, END, len_END) == 0){
                            break;
                        }
                        else{
                            size_t tmp_c = 0;
                            switch (i)
                            {
                            case 0:
                                for(size_t t = 0; t < sizeof(line) / sizeof(char); t++){
                                    if(line[t] == '\n'){
                                        line[t] = '\0';
                                        break;
                                    }
                                }
                                tmp_c = blacklist.open.count;
                                memset(blacklist.open.rules[tmp_c], 0, sizeof(blacklist.open.rules[tmp_c]));
                                strncpy(blacklist.open.rules[tmp_c], line, strlen(line));
                                blacklist.open.count++;
                                break;
                            case 1:
                                for(size_t t = 0; t < sizeof(line) / sizeof(char); t++){
                                    if(line[t] == '\n'){
                                        line[t] = '\0';
                                        break;
                                    }
                                }
                                tmp_c = blacklist.read.count;
                                memset(blacklist.read.rules[tmp_c], 0, sizeof(blacklist.read.rules[tmp_c]));
                                strncpy(blacklist.read.rules[tmp_c], line, strlen(line));
                                blacklist.read.count++;
                                break;
                            case 2:
                                for(size_t t = 0; t < sizeof(line) / sizeof(char); t++){
                                    if(line[t] == '\n'){
                                        line[t] = '\0';
                                        break;
                                    }
                                }
                                tmp_c = blacklist.write.count;
                                memset(blacklist.write.rules[tmp_c], 0, sizeof(blacklist.write.rules[tmp_c]));
                                strncpy(blacklist.write.rules[tmp_c], line, strlen(line));
                                blacklist.write.count++;
                                break;
                            case 3:
                                for(size_t t = 0; t < sizeof(line) / sizeof(char); t++){
                                    if(line[t] == '\n'){
                                        line[t] = '\0';
                                        break;
                                    }
                                }
                                tmp_c = blacklist.connect.count;
                                memset(blacklist.connect.rules[tmp_c], 0, sizeof(blacklist.connect.rules[tmp_c]));
                                strncpy(blacklist.connect.rules[tmp_c], line, strlen(line));
                                blacklist.connect.count++;
                                break;
                            case 4:
                                for(size_t t = 0; t < sizeof(line) / sizeof(char); t++){
                                    if(line[t] == '\n'){
                                        line[t] = '\0';
                                        break;
                                    }
                                }
                                tmp_c = blacklist.getaddrinfo.count;
                                memset(blacklist.getaddrinfo.rules[tmp_c], 0, sizeof(blacklist.getaddrinfo.rules[tmp_c]));
                                strncpy(blacklist.getaddrinfo.rules[tmp_c], line, strlen(line));
                                blacklist.getaddrinfo.count++;
                                break;
                            case 5:
                                for(size_t t = 0; t < sizeof(line) / sizeof(char); t++){
                                    if(line[t] == '\n'){
                                        line[t] = '\0';
                                        break;
                                    }
                                }
                                tmp_c = blacklist.system.count;
                                memset(blacklist.system.rules[tmp_c], 0, sizeof(blacklist.system.rules[tmp_c]));
                                strncpy(blacklist.system.rules[tmp_c], line, strlen(line));
                                blacklist.system.count++;
                                break;
                            default:
                                break;
                            }

                        }
                    }
                }
            }
        }
    }

    fclose(fp);

    // Print the loaded blacklist
    dprintf(debugger_fd, "number of rules of open in blacklist: %ld\n", blacklist.open.count);
    for(size_t i = 0; i < blacklist.open.count; i++){
        dprintf(debugger_fd, "%ld: %s\n", i, blacklist.open.rules[i]);
    }
    dprintf(debugger_fd, "number of rules of read in blacklist: %ld\n", blacklist.read.count);
    for(size_t i = 0; i < blacklist.read.count; i++){
        dprintf(debugger_fd, "%ld: %s\n", i, blacklist.read.rules[i]);
    }
    dprintf(debugger_fd, "number of rules of write in blacklist: %ld\n", blacklist.write.count);
    for(size_t i = 0; i < blacklist.write.count; i++){
        dprintf(debugger_fd, "%ld: %s\n", i, blacklist.write.rules[i]);
    }
    dprintf(debugger_fd, "number of rules of connect in blacklist: %ld\n", blacklist.connect.count);
    for(size_t i = 0; i < blacklist.connect.count; i++){
        dprintf(debugger_fd, "%ld: %s\n", i, blacklist.connect.rules[i]);
    }
    dprintf(debugger_fd, "number of rules of getaddrinfo in blacklist: %ld\n", blacklist.getaddrinfo.count);
    for(size_t i = 0; i < blacklist.getaddrinfo.count; i++){
        dprintf(debugger_fd, "%ld: %s\n", i, blacklist.getaddrinfo.rules[i]);
    }
    dprintf(debugger_fd, "number of rules of system in blacklist: %ld\n", blacklist.system.count);
    for(size_t i = 0; i < blacklist.system.count; i++){
        dprintf(debugger_fd, "%ld: %s\n", i, blacklist.system.rules[i]);
    }
    dprintf(debugger_fd, "\n");
}
size_t findReadFd(int fd){
    for(size_t i = 0; i < num_fd; i++){
        if(fd_read_info[i].fd == fd) return i;
    }
    return num_fd;
}
void addReadFd(int type, int fd){
    // type 0 for read, type 1 for write
    fd_read_info[num_fd].fd = fd;
    if(type == 0)
        snprintf(fd_read_info[num_fd].log_file_name, sizeof(fd_read_info[num_fd].log_file_name), "%d-%d-read.log", getpid(), fd);
    else if(type == 1)
        snprintf(fd_read_info[num_fd].log_file_name, sizeof(fd_read_info[num_fd].log_file_name), "%d-%d-write.log", getpid(), fd);

    num_fd++;
}
size_t findWriteFd(int fd){
    for(size_t i = 0; i < num_fd; i++){
        if(fd_write_info[i].fd == fd) return i;
    }
    return num_fd;
}
void addWriteFd(int type, int fd){
    // type 0 for read, type 1 for write
    fd_write_info[num_fd].fd = fd;
    if(type == 0)
        snprintf(fd_write_info[num_fd].log_file_name, sizeof(fd_write_info[num_fd].log_file_name), "%d-%d-read.log", getpid(), fd);
    else if(type == 1)
        snprintf(fd_write_info[num_fd].log_file_name, sizeof(fd_write_info[num_fd].log_file_name), "%d-%d-write.log", getpid(), fd);

    num_fd++;
}
// function to check if a file path is blocked
int checkOpenBlacklist(const char* file_path){
    blacklist_t *current_function = NULL;
    char file_realpath[1000];

    current_function = &blacklist.open;
    realpath(file_path, file_realpath);
    
    for (int i = 0; i < current_function->count; i++) {
        char *current_rules = current_function->rules[i];
        realpath(current_function->rules[i], current_rules);
        if (strcmp(file_realpath, current_rules) == 0) {
            return 1;
        }
    }
    return 0;
}

int checkConnectBlacklist(const struct sockaddr *addr) {
    char ip[INET6_ADDRSTRLEN];
    uint16_t port;

    switch (addr->sa_family) {
        case AF_INET: {
            struct sockaddr_in *addr_in4 = (struct sockaddr_in *)addr;
            void *sin_addr = &(addr_in4->sin_addr);
            inet_ntop(AF_INET, sin_addr, ip, INET_ADDRSTRLEN);
            port = ntohs(addr_in4->sin_port);
            break;
        }
        case AF_INET6: {
            struct sockaddr_in6 *addr_in6 = (struct sockaddr_in6 *)addr;
            void *sin_addr = &(addr_in6->sin6_addr);
            inet_ntop(AF_INET6, sin_addr, ip, INET6_ADDRSTRLEN);
            port = ntohs(addr_in6->sin6_port);
            break;
        }
        default: {
            return 0;
        }
    }


    for (size_t i = 0; i < blacklist.connect.count; i++) {
        char domain[256];
        uint16_t blacklist_port;
        sscanf(blacklist.connect.rules[i], "%255[^:]:%hu", domain, &blacklist_port);

        if (port != blacklist_port) {
            continue;
        }

        struct addrinfo hints, *res;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = addr->sa_family;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;

        if (getaddrinfo(domain, NULL, &hints, &res) != 0) {
            continue;
        }

        for (struct addrinfo *p = res; p != NULL; p = p->ai_next) {
            char blacklist_ip[INET6_ADDRSTRLEN];
            if (p->ai_family == AF_INET) {
                struct sockaddr_in *addr_in = (struct sockaddr_in *)p->ai_addr;
                inet_ntop(AF_INET, &(addr_in->sin_addr), blacklist_ip, INET_ADDRSTRLEN);
            }
            else if (p->ai_family == AF_INET6) {
                struct sockaddr_in6 *addr_in6 = (struct sockaddr_in6 *)p->ai_addr;
                inet_ntop(AF_INET6, &(addr_in6->sin6_addr), blacklist_ip, INET6_ADDRSTRLEN);
            }
            else {
                continue;
            }
            if (strcmp(ip, blacklist_ip) == 0) {
                freeaddrinfo(res);
                return 1;
            }
        }

        freeaddrinfo(res);
    }

    return 0;
}
