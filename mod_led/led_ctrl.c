#include <linux/cdev.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/kernfs.h>
#include <linux/module.h>
#include <linux/timer.h>
#include <linux/types.h>
#include <linux/uaccess.h>

#define GPIO_NUM (43)  // 待操作的gpio管脚
// ioctl命令行
#define LED_IOC_MAGIC 'L'
#define SET_LED_OFF _IO(LED_IOC_MAGIC, 1)  // 开灯指令
#define SET_LED_ON _IO(LED_IOC_MAGIC, 2)   // 关灯指令

enum led_stat {
    LED_OFF = 0x0,
    LED_ON,
};

// 假设有个全局的内核资源
static int g_data = 5;

// 文件打开函数
int led_open(struct inode *inode, struct file *filp) {
    return 0;  // 默认成功打开
}

/*文件释放函数*/
int led_release(struct inode *inode, struct file *filp) {
    return 0;  // 默认成功打开
}

/*读函数*/
static ssize_t led_read(struct file *filp, char __user *buf, size_t size, loff_t *ppos) {
    // 将内核空间的数据copy到用户空间
    if (copy_to_user(buf, &g_data, size)) {
        return -EFAULT;
    }

    return size;
}

/*写函数*/
static ssize_t led_write(struct file *filp, const char __user *buf, size_t size, loff_t *ppos) {
    // 将用户空间数据copy到内核空间
    if (copy_from_user(&g_data, buf, size)) {
        return -EFAULT;
    }

    return size;
}

static long led_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {
    long ret = 0;

    // check IOCTL command magic
    if (_IOC_TYPE(cmd) != LED_IOC_MAGIC) {
        printk("[%s] invalid arg!\n", __func__);
        return -ESRCH;
    }

    switch (cmd) {
        case SET_LED_ON:
            // 点亮该led
            gpio_direction_output(GPIO_NUM, LED_ON);
            break;
        case SET_LED_OFF:
            // 熄灭该led
            gpio_direction_output(GPIO_NUM, LED_OFF);
            break;
        default:
            ret = -ESRCH;
    }

    return ret;
}

/*文件操作结构体*/
static const struct file_operations hello_fops =
    {
        .owner = THIS_MODULE,
        .open = led_open,             // 对应 open
        .read = led_read,             // 对应 read
        .write = led_write,           // 对应 write
        .unlocked_ioctl = led_ioctl,  // 对应 ioctl
        .release = led_release,       // 对应 close
};

struct cdev cdev;  // 静态定义字符设备cdev结构
dev_t devno;       // 静态定义字符设备号

// 设备驱动模块加载函数
static int __init helloworld_init(void) {
    int ret = -1;
    unsigned int *addr;

    // 初始化cdev结构，并将文件IO与字符设备的cdev建立链接
    cdev_init(&cdev, &hello_fops);
    // 动态申请设备号
    ret = alloc_chrdev_region(&devno, 0, 1, "led_ctrl");
    if (ret) {
        printk("alloc_chrdev_region error: %d\n", ret);
        cdev_del(&cdev);  // 注销设备
        return ret;
    }
    // 向 linux 系统添加一个字符设备
    cdev_add(&cdev, devno, 1);

    // 初始化 gpio 管脚
    // 修改gpio42 gpio43 gpio52 gpio53管脚复用关系为gpio模式
    addr = ioremap(0x04020068, 4);
    writel(0x01001000, addr);
    writel(0x01001000, addr + 1);
    writel(0x01001000, addr + 2);
    writel(0x01001000, addr + 3);
    iounmap(addr);
    // 申请 led 一个 gpio
    ret = gpio_request(GPIO_NUM, "led_gpio");
    if (ret < 0) {
        printk("gpio_request[%d] error:%d\n", GPIO_NUM, ret);
        goto ERR;
    }
    // 默认关闭该 led
    gpio_direction_output(GPIO_NUM, LED_OFF);
    printk("##request gpio suncess!\n");

    printk("helloworld init\n");

    return 0;

ERR:
    cdev_del(&cdev);                     // 注销设备
    unregister_chrdev_region(devno, 1);  // 释放设备号

    return ret;
}

// 模块卸载函数
static void __exit helloworld_exit(void) {
    cdev_del(&cdev);                           // 注销设备
    unregister_chrdev_region(devno, 1);        // 释放设备号

    gpio_direction_output(GPIO_NUM, LED_OFF);  // 默认熄灭该 led
    gpio_free(GPIO_NUM);

    printk("helloworld exit\n");
}

module_init(helloworld_init);  // 内核模块加载函数
module_exit(helloworld_exit);  // 内核模块卸载函数
MODULE_LICENSE("GPL");
MODULE_AUTHOR("zhangsan");
MODULE_DESCRIPTION("this is a led control module");
