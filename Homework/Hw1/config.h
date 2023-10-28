#include <arpa/inet.h>
#ifndef CONFIG_H
#define CONFIG_H
#define MAX_RULES 1000
#define MAX_FD 1024
typedef struct {
    char rules[MAX_RULES][1024];
    size_t count;
} blacklist_t;

typedef struct {
    blacklist_t open;
    blacklist_t read;
    blacklist_t write;
    blacklist_t connect;
    blacklist_t getaddrinfo;
    blacklist_t system;
} api_function_blacklist_t;

typedef struct {
    int fd;
    char log_file_name[256];
    char content[1000];
} fd_info_t;

extern api_function_blacklist_t blacklist;
extern fd_info_t fd_read_info[MAX_FD];
extern fd_info_t fd_write_info[MAX_FD];
extern size_t num_fd;
extern int logger_fd;
extern int debugger_fd;
// function to load the config file and populate the blocked_files array
void loadConfigFile(char *api_function_list[], size_t api_function_list_size);
size_t findReadFd(int fd);
void addReadFd(int type, int fd);
size_t findWriteFd(int fd);
void addWriteFd(int type, int fd);
int checkOpenBlacklist(const char* file_path);
int checkConnectBlacklist(const struct sockaddr *addr);
#endif // CONFIG_H