#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>

#define S3C2440_GPA(n) (0<<16 | n)
#define S3C2440_GPb(n) (1<<16 | n)
#define S3C2440_GPC(n) (2<<16 | n)
#define S3C2440_GPD(n) (3<<16 | n)
#define S3C2440_GPE(n) (4<<16 | n)
#define S3C2440_GPF(n) (5<<16 | n)
#define S3C2440_GPG(n) (6<<16 | n)
#define S3C2440_GPH(n) (7<<16 | n)
#define S3C2440_GPI(n) (8<<16 | n)
#define S3C2440_GPJ(n) (9<<16 | n)

static int led_pin = S3C2440_GPF(5);

static volatile unsigned int *gpio_con;
static volatile unsigned int *gpio_dat;

static struct class *led_class;
static struct class_device *led_class_dev;

/*
 * 123. 分配/设置/注册file_operations
 * 4. 入口
 * 5. 出口
 */

static int major;

/* 根据芯片手册 */
static unsigned int gpio_base[] = {
    0x56000000, /* GPACON */
    0x56000010, /* GPBCON */
    0x56000020, /* GPCCON */
    0x56000030, /* GPDCON */
    0x56000040, /* GPECON */
    0x56000050, /* GPFCON */
    0x56000060, /* GPGCON */
    0x56000070, /* GPHCON */
    0,          /* GPICON */
    0x560000D0, /* GPJCON */
};

static int led_open (struct inode *node, struct file *filp)
{
    /* 把LED引脚配置为输出引脚 */
    /* GPF5 - 0x56000050 */
    int bank = led_pin >> 16;
    int base = gpio_base[bank];

    int pin = led_pin & 0xFFFF;

    gpio_con = ioremap(base, 8);
    if (gpio_con) {
        printk("ioremap(0x%x) = 0x%x\n", base, gpio_con);
    } else {
        return -EINVAL;
    }

    base += 4;
    gpio_dat = ioremap(base, 8);
    if (gpio_dat) {
        printk("ioremap(0x%x) = 0x%x\n", base, gpio_dat);
    } else {
        return -EINVAL;
    }

    *gpio_con &= ~(0x03 << (pin * 2));
    *gpio_con |=  (0x01 << (pin * 2));

    return 0;
}

// 第二个参数中的__usr是一个空的宏，表示该参数来自应用程序，不可以直接使用
static ssize_t led_write (struct file *filp, const char __user *buf, size_t size, loff_t *off)
{
    /* 根据应用程序传入的值来设置LED引脚 */
    unsigned char val;
    int pin = led_pin & 0xffff;

    copy_from_user(&val, buf, 1);

    if (val) {
        /* 点灯 */
        *gpio_dat &= ~(1 << pin);
    } else {
        /* 灭灯 */
        *gpio_dat |=  (1 << pin);
    }

    return 1;   /* 已写入1个数据 */
}


static int led_release (struct inode *node, struct file *filp)
{
    printk("iounmap(gpio_con) = 0x%x\n", gpio_con);
    printk("iounmap(gpio_dat) = 0x%x\n", gpio_dat);

    iounmap(gpio_con);
    iounmap(gpio_dat);

    gpio_con = NULL;
    gpio_dat = NULL;

    return 0;
}

static struct file_operations myled_oprs = {
    .owner      = THIS_MODULE,
    .open       = led_open,
    .write      = led_write,
    .release    = led_release,
};

static int myled_init(void)
{
    major = register_chrdev(0, "myled", &myled_oprs);

    led_class = class_create(THIS_MODULE, "myled");

    led_class_dev = device_create(led_class, NULL, MKDEV(major, 0), NULL, "led"); /* /dev/led */

    return 0;
}

static void myled_exit(void)
{
    unregister_chrdev(major, "myled");

    device_destroy(led_class, MKDEV(major, 0));

    class_destroy(led_class);

    if (gpio_dat != NULL) {
        iounmap(gpio_dat);
        gpio_dat = NULL;
    }

    if (gpio_con != NULL) {
        iounmap(gpio_con);
        gpio_con = NULL;
    }
}

module_init(myled_init);
module_exit(myled_exit);

MODULE_LICENSE("GPL");

