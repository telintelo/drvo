#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/fs.h>

#define MAX_DEV 2

static int opdracht3_open(struct inode *inode, struct file *file);
static int opdracht3_release(struct inode *inode, struct file *file);
static long opdracht3_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
static ssize_t opdracht3_read(struct file *file, char __user *buf, size_t count, loff_t *offset);
static ssize_t opdracht3_write(struct file *file, const char __user *buf, size_t count, loff_t *offset);

static const struct file_operations opdracht3_fops = {
    .owner = THIS_MODULE,
    .open = opdracht3_open,
    .release = opdracht3_release,
    .unlocked_ioctl = opdracht3_ioctl,
    .read = opdracht3_read,
    .write = opdracht3_write
};

struct mychar_device_data {
    struct cdev cdev;
};

static int dev_major = 0;
static struct class *opdracht3_class = NULL;
static struct mychar_device_data opdracht3_data[MAX_DEV];

static int opdracht3_uevent(struct device *dev, struct kobj_uevent_env *env)
{
    add_uevent_var(env, "DEVMODE=%#o", 0666);
    return 0;
}

static int opdracht3_init(void)
{
    int err, i;
    dev_t dev;

    err = alloc_chrdev_region(&dev, 0, MAX_DEV, "opdracht3");

    dev_major = MAJOR(dev);

    opdracht3_class = class_create(THIS_MODULE, "opdracht3");
    opdracht3_class->dev_uevent = opdracht3_uevent;

    for (i = 0; i < MAX_DEV; i++) {
        cdev_init(&opdracht3_data[i].cdev, &opdracht3_fops);
        opdracht3_data[i].cdev.owner = THIS_MODULE;

        cdev_add(&opdracht3_data[i].cdev, MKDEV(dev_major, i), 1);

        device_create(opdracht3_class, NULL, MKDEV(dev_major, i), NULL, "opdracht3-%d", i);
    }

    return 0;
}

static void opdracht3_exit(void)
{
    int i;

    for (i = 0; i < MAX_DEV; i++) {
        device_destroy(opdracht3_class, MKDEV(dev_major, i));
    }

    class_unregister(opdracht3_class);
    class_destroy(opdracht3_class);

    unregister_chrdev_region(MKDEV(dev_major, 0), MINORMASK);
}

static int opdracht3_open(struct inode *inode, struct file *file)
{
    printk("opdracht3: Device open\n");
    return 0;
}

static int opdracht3_release(struct inode *inode, struct file *file)
{
    printk("opdracht3: Device close\n");
    return 0;
}

static long opdracht3_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    printk("opdracht3: Device ioctl\n");
    return 0;
}

static ssize_t opdracht3_read(struct file *file, char __user *buf, size_t count, loff_t *offset)
{
    uint8_t *data = "Hello from the kernel world!\n";
    size_t datalen = strlen(data);

    printk("Reading device: %d\n", MINOR(file->f_path.dentry->d_inode->i_rdev));

    if (count > datalen) {
        count = datalen;
    }

    if (copy_to_user(buf, data, count)) {
        return -EFAULT;
    }

    return count;
}

static ssize_t opdracht3_write(struct file *file, const char __user *buf, size_t count, loff_t *offset)
{
    size_t maxdatalen = 30, ncopied;
    uint8_t databuf[maxdatalen];

    printk("Writing device: %d\n", MINOR(file->f_path.dentry->d_inode->i_rdev));

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

module_init(opdracht3_init);
module_exit(opdracht3_exit);
