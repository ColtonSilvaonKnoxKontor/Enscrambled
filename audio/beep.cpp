#include "beep.hpp"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/kd.h>
#include <iostream>

bool pc_beep() {
    int fd = open("/dev/console", O_WRONLY);
    if (fd == -1) {
        // Fallback to trying a tty
        fd = open("/dev/tty0", O_WRONLY);
        if (fd == -1) {
            perror("pc_beep: open");
            return false;
        }
    }

    // Beep with a frequency of 880 Hz for 200 ms
    if (ioctl(fd, KIOCSOUND, (int)(1193180 / 880)) == -1) {
        perror("pc_beep: ioctl");
        close(fd);
        return false;
    }

    usleep(200 * 1000); // Beep duration

    // Stop the beep
    if (ioctl(fd, KIOCSOUND, 0) == -1) {
        perror("pc_beep: ioctl");
        close(fd);
        return false;
    }

    close(fd);
    return true;
}
