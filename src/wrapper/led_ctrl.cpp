#include "led_ctrl.h"

#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define LED_IOC_MAGIC 'L'
#define SET_LED_OFF _IO(LED_IOC_MAGIC, 1)
#define SET_LED_ON _IO(LED_IOC_MAGIC, 2)

LedCtrl::LedCtrl() {
    fd = open("/dev/led_ctrl", O_RDWR);
    if (fd < 0) {
        perror("open");
    }
}

LedCtrl::~LedCtrl() {
    close(fd);
}

void LedCtrl::on() {
    ioctl(fd, SET_LED_ON);
}

void LedCtrl::off() {
    ioctl(fd, SET_LED_OFF);
}
