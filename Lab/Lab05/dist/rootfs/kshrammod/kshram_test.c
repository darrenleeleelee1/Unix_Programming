#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "kshram.h"

#define DEVFILE "/dev/kshram0"

int main() {
    int fd;
    int slots;
    int size, new_size = 1052672;

    if ((fd = open(DEVFILE, O_RDWR)) < 0) {
        perror("open");
        return -1;
    }

    // Test KSHRAM_GETSLOTS
    slots = ioctl(fd, KSHRAM_GETSLOTS);
    if (slots >= 0) {
        printf("Number of slots available: %d\n", slots);
    } else {
        perror("KSHRAM_GETSLOTS");
    }

    // Test KSHRAM_GETSIZE
    size = ioctl(fd, KSHRAM_GETSIZE);
    if (size != (size_t)-1) {
        printf("Current memory size: %d\n", size);
    } else {
        perror("KSHRAM_GETSIZE");
    }

    // Test KSHRAM_SETSIZE
    if (ioctl(fd, KSHRAM_SETSIZE, new_size) == 0) {
        printf("Memory size set to: %d\n", new_size);
    } else {
        perror("KSHRAM_SETSIZE");
    }

    close(fd);

    return 0;
}
