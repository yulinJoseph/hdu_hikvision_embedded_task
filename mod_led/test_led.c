#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/* ioctl命令行 */
#define LED_IOC_MAGIC 'L'
#define SET_LED_OFF _IO(LED_IOC_MAGIC, 1)
#define SET_LED_ON _IO(LED_IOC_MAGIC, 2)

void usage(void) {
    printf("#################################\n");
    printf("please input:on|off|quit\n");
    printf("#################################\n");
}
int main(int argc, char *argv[]) {
    char buf[1024] = {0};

    // 打开设备驱动节点
    int fd = open("/dev/led_ctrl", O_RDWR);
    if (fd < 0) {
        perror("open");
        return -1;
    }

    while (fgets(buf, 1024, stdin) != NULL) {
        printf("##$:\n");
        if (0 == strncmp(buf, "on", 2)) {
            ioctl(fd, SET_LED_ON);
            continue;
        } else if (0 == strncmp(buf, "off", 3)) {
            ioctl(fd, SET_LED_OFF);
            continue;
        } else if (0 == strncmp(buf, "quit", 4)) {
            goto end;
        } else {
            usage();
        }
        memset(buf, 0, 1024);
    }

end:
    close(fd);
    return 0;
}
