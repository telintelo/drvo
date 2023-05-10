#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/fs.h>

#define MAX_DEV 2
#define G_BUFFER_SIZE 64

int my_parameter = 0;
module_param(my_parameter, int, S_IRUSR|S_IWUSR);

static int opdracht4_open(struct inode *inode, struct file *file);
static int opdracht4_release(struct inode *inode, struct file *file);
static long opdracht4_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
static ssize_t opdracht4_read(struct file *file, char __user *buf, size_t count, loff_t *offset);
static ssize_t opdracht4_write(struct file *file, const char __user *buf, size_t count, loff_t *offset);

static const struct file_operations opdracht4_fops = {
    .owner = THIS_MODULE,
    .open = opdracht4_open,
    .release = opdracht4_release,
    .unlocked_ioctl = opdracht4_ioctl,
    .read = opdracht4_read,
    .write = opdracht4_write
};

struct opdracht4_device_data {
    struct cdev cdev;
};

static int dev_major = 0;
static struct class *opdracht4_class = NULL;
static struct opdracht4_device_data opdracht4_data[MAX_DEV];

static int device_opened = 0;
static char g_buffer[G_BUFFER_SIZE];

static int opdracht4_uevent(struct device *dev, struct kobj_uevent_env *env)
{
    printk("static int opdracht4_uevent(struct device *dev, struct kobj_uevent_env *env) called.\n");
    add_uevent_var(env, "DEVMODE=%#o", 0666);
    return 0;
}

static int opdracht4_init(void)
{
    int err, i;
    dev_t dev;

    printk("static int opdracht4_init(void) called.\n");
    printk("Parameter: %d\n", my_parameter);

    err = alloc_chrdev_region(&dev, 0, MAX_DEV, "opdracht4");

    dev_major = MAJOR(dev);

    opdracht4_class = class_create(THIS_MODULE, "opdracht4");
    opdracht4_class->dev_uevent = opdracht4_uevent;

    for (i = 0; i < MAX_DEV; i++) {
        cdev_init(&opdracht4_data[i].cdev, &opdracht4_fops);
        opdracht4_data[i].cdev.owner = THIS_MODULE;

        cdev_add(&opdracht4_data[i].cdev, MKDEV(dev_major, i), 1);
    
        device_create(opdracht4_class, NULL, MKDEV(dev_major, i), NULL, "opdracht4-%d", i);
    }

    memset(&g_buffer[0], 'a', G_BUFFER_SIZE);
    g_buffer[G_BUFFER_SIZE] = '\0';
    g_buffer[G_BUFFER_SIZE - 1] = 'C';

    return 0;
}

static void opdracht4_exit(void)
{
    int i;

    printk("static void opdracht4_exit(void) called.\n");

    for (i = 0; i < MAX_DEV; i++) {
        device_destroy(opdracht4_class, MKDEV(dev_major, i));
        cdev_del(&opdracht4_data[i].cdev);
    }

    class_unregister(opdracht4_class);
    class_destroy(opdracht4_class);

    unregister_chrdev_region(MKDEV(dev_major, 0), MINORMASK);
}

static int opdracht4_open(struct inode *inode, struct file *file)
{
    printk("static int opdracht4_open(struct inode *inode, struct file *file) called.\n");
    device_opened++;
    printk("Device opened %d times.\n", device_opened);
    return 0;
}

static int opdracht4_release(struct inode *inode, struct file *file)
{
    printk("static int opdracht4_release(struct inode *inode, struct file *file) called.\n");
    return 0;
}

static long opdracht4_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    printk("static long opdracht4_ioctl(struct file *file, unsigned int cmd, unsigned long arg) called.\n");
    return 0;
}

static ssize_t opdracht4_read(struct file *file, char __user *buf, size_t count, loff_t *offset)
{
    printk("static ssize_t opdracht4_read(struct file *file, char __user *buf, size_t count, loff_t *offset) called with:\n");
    printk("size_t count = %d, loff_t *offset = %d.\n", count, *offset);

    printk("Reading device: %d\n", MINOR(file->f_path.dentry->d_inode->i_rdev));

    if (count + *offset > G_BUFFER_SIZE) {
        count = G_BUFFER_SIZE - *offset;
    }

    if (copy_to_user(buf, g_buffer + *offset, count)) {
        printk("Copy to user returned an error.\n");
        return -EFAULT;
    }

    printk("Returned %d\n", count);

    return count;
}

static ssize_t opdracht4_write(struct file *file, const char __user *buf, size_t count, loff_t *offset)
{
    printk("static ssize_t opdracht4_write(struct file *file, const char __user *buf, size_t count, loff_t *offset) called.\n");
    printk("size_t count = %d, loff_t *offset = %d.\n", count, *offset);

    printk("Writing device: %d\n", MINOR(file->f_path.dentry->d_inode->i_rdev));

    if (count + *offset > G_BUFFER_SIZE) {
        count = G_BUFFER_SIZE - *offset;
    }

    if (copy_from_user(g_buffer + *offset, buf, count)) {
        printk("Copy from user returned an error.\n");
        return -EFAULT;
    }

    printk("Copied %zd bytes from the user\n", count);

    printk("New buffer state: %s\n", g_buffer);

    return count;
}

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Max te Lintelo <max.te.lintelo@gmail.com>");

module_init(opdracht4_init);
module_exit(opdracht4_exit);
