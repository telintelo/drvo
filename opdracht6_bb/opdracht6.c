#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/io.h>

#define MAX_DEV 2

#define PIN 19
#define GPIO1_ADDR 0x4804c000
#define GPIO_OE 0x4D // 0x134
#define GPIO_DATAIN 0x4E // 0x138
#define GPIO_CLEARDATAOUT 0x64 // 0x190
#define GPIO_SETDATAOUT 0x65 // 0x194
#define GPIO_MAX 0x198

uint32_t* gpio1;
uint32_t oe;

int my_parameter;
module_param(my_parameter, int, S_IRUSR|S_IWUSR);

static int opdracht6_open(struct inode *inode, struct file *file);
static int opdracht6_release(struct inode *inode, struct file *file);
static long opdracht6_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
static ssize_t opdracht6_read(struct file *file, char __user *buf, size_t count, loff_t *offset);
static ssize_t opdracht6_write(struct file *file, const char __user *buf, size_t count, loff_t *offset);

static const struct file_operations opdracht6_fops = {
    .owner = THIS_MODULE,
    .open = opdracht6_open,
    .release = opdracht6_release,
    .unlocked_ioctl = opdracht6_ioctl,
    .read = opdracht6_read,
    .write = opdracht6_write
};

struct opdracht6_device_data {
    struct cdev cdev;
};

static int dev_major = 0;
static struct class *opdracht6_class = NULL;
static struct opdracht6_device_data opdracht6_data[MAX_DEV];

static int opdracht6_uevent(struct device *dev, struct kobj_uevent_env *env)
{
    add_uevent_var(env, "DEVMODE=%#o", 0666);
    return 0;
}

static int opdracht6_init(void)
{
    int err, i;
    dev_t dev;

    printk("opdracht6_init() called.");
    printk("Parameter: %d\n", my_parameter);

    err = alloc_chrdev_region(&dev, 0, MAX_DEV, "opdracht6");

    dev_major = MAJOR(dev);

    opdracht6_class = class_create(THIS_MODULE, "opdracht6");
    opdracht6_class->dev_uevent = opdracht6_uevent;

    for (i = 0; i < MAX_DEV; i++) {
        cdev_init(&opdracht6_data[i].cdev, &opdracht6_fops);
        opdracht6_data[i].cdev.owner = THIS_MODULE;

        cdev_add(&opdracht6_data[i].cdev, MKDEV(dev_major, i), 1);
    
        device_create(opdracht6_class, NULL, MKDEV(dev_major, i), NULL, "opdracht6-%d", i);
    }

    gpio1 = ioremap( GPIO1_ADDR, GPIO_MAX * sizeof(uint32_t) );
    barrier();
    oe = ioread32( gpio1 + GPIO_OE );
    rmb();
    iowrite32( (oe & (~(1<<PIN))), gpio1 + GPIO_OE );
    wmb(); // write memory barrier
    iounmap(gpio1);

    return 0;
}

static void opdracht6_exit(void)
{
    int i;

    for (i = 0; i < MAX_DEV; i++) {
        device_destroy(opdracht6_class, MKDEV(dev_major, i));
        cdev_del(&opdracht6_data[i].cdev);
    }

    class_unregister(opdracht6_class);
    class_destroy(opdracht6_class);

    unregister_chrdev_region(MKDEV(dev_major, 0), MINORMASK);
}

static int opdracht6_open(struct inode *inode, struct file *file)
{
    printk("opdracht6: Device open\n");
    return 0;
}

static int opdracht6_release(struct inode *inode, struct file *file)
{
    printk("opdracht6: Device close\n");
    return 0;
}

static long opdracht6_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    printk("opdracht6: Device ioctl\n");
    return 0;
}

static ssize_t opdracht6_read(struct file *file, char __user *buf, size_t count, loff_t *offset)
{
    uint8_t *data = "Hello from kernel-space!\n";
    size_t datalen = strlen(data);

    printk("Reading device: %d\n", MINOR(file->f_path.dentry->d_inode->i_rdev));

    gpio1 = ioremap(GPIO1_ADDR, GPIO_MAX * sizeof(uint32_t));
    barrier();
    iowrite32( (1<<PIN), gpio1 + GPIO_CLEARDATAOUT ); // Pin 19 uit
    wmb(); // write memory barrier
    iounmap(gpio1);

    if (count > datalen) {
        count = datalen;
    }

    if (copy_to_user(buf, data, count)) {
        return -EFAULT;
    }

    return count;
}

static ssize_t opdracht6_write(struct file *file, const char __user *buf, size_t count, loff_t *offset)
{
    size_t maxdatalen = 30, ncopied;
    uint8_t databuf[maxdatalen];

    printk("Writing device: %d\n", MINOR(file->f_path.dentry->d_inode->i_rdev));

    gpio1 = ioremap(GPIO1_ADDR, GPIO_MAX * sizeof(uint32_t));
    barrier();
    iowrite32( (1<<PIN), gpio1 + GPIO_SETDATAOUT ); // Pin 19 aan
    wmb(); // write memory barrier
    iounmap(gpio1);

    if (count < maxdatalen) {
        maxdatalen = count;
    }

    ncopied = copy_from_user(databuf, buf, maxdatalen);

    if (ncopied == 0) {
        printk("Copied %zd bytes from the user\n", maxdatalen);
    } else {
        printk("Could't copy %zd bytes from the user\n", ncopied);
    }

    databuf[maxdatalen] = 0;

    printk("Data from the user: %s\n", databuf);

    return count;
}

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Max te Lintelo <max.te.lintelo@gmail.com>");

module_init(opdracht6_init);
module_exit(opdracht6_exit);
