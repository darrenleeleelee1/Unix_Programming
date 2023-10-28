#ifndef FUNCTION_LIST_H
#define FUNCTION_LIST_H
#include <netdb.h>
#include <stddef.h>
#include <sys/stat.h>
#include <sys/types.h>

int hijackopen(const char *pathname, int flags, mode_t mode);
ssize_t hijackread(int fd, void *buf, size_t count);
ssize_t hijackwrite(int  fd,  const  void  *buf, size_t count);
int hijackconnect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int hijackgetaddrinfo(const char *node, const char *service,
            const struct addrinfo *hints, struct addrinfo **res);
int hijacksystem(const char *command);
#endif // FUNCTION_LIST_H
