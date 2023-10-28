#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "function_list.h"
#include "config.h"
#define PATH_MAX 1000
int hijackopen(const char *pathname, int flags, mode_t mode){
    dprintf(debugger_fd, "\n---HIJACK open---\n");
    
    if ((flags & O_CREAT) == 0) mode = 0;
    // Print the loaded blacklist
    int check = checkOpenBlacklist(pathname);
    if(check == 1) {
        errno = EACCES;
        dprintf(logger_fd, "[logger] open(\"%s\", %d, %hu) = %d\n", pathname, flags, mode, -1);
        return -1;
    }
    int (*original_open)(const char*, int, mode_t) = dlsym(RTLD_NEXT, "open");

    int original_return_value = original_open(pathname, flags, mode);
    dprintf(logger_fd, "[logger] open(\"%s\", %d, %hu) = %d\n", pathname, flags, mode, original_return_value);
    return original_return_value;
}
ssize_t hijackread(int fd, void *buf, size_t count){
    dprintf(debugger_fd, "\n---HIJACK read---\n");
    ssize_t (*original_read)(int, void *, size_t) = dlsym(RTLD_NEXT, "read");

    size_t fd_index = findReadFd(fd);
    if(fd_index == num_fd) addReadFd(0, fd);
    // Call original `read`
    ssize_t original_return_value = original_read(fd, buf, count);
    int read_fd_log = open(fd_read_info[fd_index].log_file_name, O_WRONLY | O_CREAT | O_APPEND, 0666);
    if (read_fd_log == -1) {
        dprintf(2, "Error opening log file %s", fd_read_info[fd_index].log_file_name);
        return original_return_value;
    }
    size_t content_length = strlen(fd_read_info[fd_index].content);
    memcpy(fd_read_info[fd_index].content + content_length, buf, original_return_value);
    fd_read_info[fd_index].content[content_length + original_return_value] = '\0'; // Null-terminate the content
    for(size_t i = 0; i < blacklist.read.count; i++){
        if(strstr(fd_read_info[fd_index].content, blacklist.read.rules[i]) != NULL) {
            close(fd_read_info[fd_index].fd);
            close(read_fd_log);
            errno = EIO;
            dprintf(logger_fd, "[logger] read(%d, %p, %zu) = %d\n", fd, buf, count, -1);
            return -1;
        }
    }
    write(read_fd_log, buf, original_return_value);
    close(read_fd_log);

    dprintf(logger_fd, "[logger] read(%d, %p, %zu) = %ld\n", fd, buf, count, original_return_value);
    return original_return_value;
}
ssize_t hijackwrite(int  fd,  const  void  *buf, size_t count){
    dprintf(debugger_fd, "\n---HIJACK write---\n");
    ssize_t (*original_write)(int, const void *, size_t) = dlsym(RTLD_NEXT, "write");

    size_t fd_index = findWriteFd(fd);
    if(fd_index == num_fd) addWriteFd(1, fd);
    ssize_t original_return_value = original_write(fd, buf, count);
    int write_fd_log = open(fd_write_info[fd_index].log_file_name, O_WRONLY | O_CREAT | O_APPEND, 0666);
    if (write_fd_log == -1) {
        dprintf(2, "Error opening log file %s", fd_write_info[fd_index].log_file_name);
        return original_return_value;
    }
    dprintf(logger_fd, "[logger] write(%d, %p, %zu) = %ld\n", fd, buf, count, original_return_value);
    write(write_fd_log, buf, count); // use the write system call to write the log file
    close(write_fd_log); // close the log file after writing
    return original_return_value;
}
int hijackconnect(int sockfd, const struct sockaddr *addr, socklen_t addrlen){
    dprintf(debugger_fd, "\n---HIJACK connect---\n");
    int (*original_connect)(int, const struct sockaddr *, socklen_t) = dlsym(RTLD_NEXT, "connect");
    int original_return_value = original_connect(sockfd, addr, addrlen);

    char ip_address[INET6_ADDRSTRLEN];
    struct sockaddr_in *ipv4_addr;
    struct sockaddr_in6 *ipv6_addr;

    if (addr->sa_family == AF_INET) {
        ipv4_addr = (struct sockaddr_in *)addr;
        inet_ntop(AF_INET, &(ipv4_addr->sin_addr), ip_address, INET_ADDRSTRLEN);
    }
    else if (addr->sa_family == AF_INET6) {
        ipv6_addr = (struct sockaddr_in6 *)addr;
        inet_ntop(AF_INET6, &(ipv6_addr->sin6_addr), ip_address, INET6_ADDRSTRLEN);
    }

    if (checkConnectBlacklist(addr)) {
        errno = ECONNREFUSED;
        dprintf(logger_fd, "[logger] connect(%d, \"%s\", %u) = -1\n", sockfd, ip_address, addrlen);
        return -1;
    }

    dprintf(logger_fd, "[logger] connect(%d, \"%s\", %u) = %d\n", sockfd, ip_address, addrlen, original_return_value);

    return original_return_value;
}
int hijackgetaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res) {
    dprintf(debugger_fd, "\n---HIJACK getaddrinfo---\n");
    int (*original_getaddrinfo)(const char *, const char *, const struct addrinfo *, struct addrinfo **) = dlsym(RTLD_NEXT, "getaddrinfo");
    int original_return_value = original_getaddrinfo(node, service, hints, res);

    // Check if the node is in the getaddrinfo blacklist
    for (size_t i = 0; i < blacklist.getaddrinfo.count; i++) {
        if (strcmp(node, blacklist.getaddrinfo.rules[i]) == 0) {
            dprintf(logger_fd, "[logger] getaddrinfo(\"%s\", \"%s\", %p, %p) = -2\n", node, service, hints, res);
            errno = EAI_NONAME;
            return EAI_NONAME;
        }
    }
    dprintf(logger_fd, "[logger] getaddrinfo(\"%s\", \"%s\", %p, %p) = %d\n", node, service, hints, res, original_return_value);


    return original_return_value;
}

int hijacksystem(const char *command) {
    dprintf(debugger_fd, "\n---HIJACK system---\n");
    int (*original_system)(const char *) = dlsym(RTLD_NEXT, "system");

    char command_copy[PATH_MAX];
    strncpy(command_copy, command, PATH_MAX - 1);
    command_copy[PATH_MAX - 1] = '\0';

    char *cmd = strtok(command_copy, " ");
    if (cmd != NULL) {
        char resolved_path[PATH_MAX];
        if (realpath(cmd, resolved_path) != NULL) {
            char launcher_path[PATH_MAX];
            realpath("./launcher", launcher_path);

            if (strcmp(resolved_path, launcher_path) == 0) {
                perror("The launcher should not be invoked again. Aborting.\n");
                return -1;
            }
        }
    }

    dprintf(logger_fd, "[logger] system(\"%s\")\n", command);
    // Call the original system function
    return original_system(command);
}