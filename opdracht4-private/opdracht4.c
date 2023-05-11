#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/fs.h>

#define MAX_DEV 2
#define QUANTUM_NUM 4000
#define QSET_SIZE 1000

int my_parameter = 0;
module_param(my_parameter, int, S_IRUSR | S_IWUSR);

static int opdracht4_open(struct inode *inode, struct file *file);
static int opdracht4_release(struct inode *inode, struct file *file);
static long opdracht4_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
static ssize_t opdracht4_read(struct file *file, char __user *buf, size_t count, loff_t *offset);
static ssize_t opdracht4_write(struct file *file, const char __user *buf, size_t count, loff_t *offset);
static loff_t opdracht4_llseek(struct file *filp, loff_t off, int whence);

static const struct file_operations opdracht4_fops = {
	.owner = THIS_MODULE,
	.llseek = opdracht4_llseek,
	.open = opdracht4_open,
	.release = opdracht4_release,
	.unlocked_ioctl = opdracht4_ioctl,
	.read = opdracht4_read,
	.write = opdracht4_write};

struct scull_qset
{
	void **data;
	struct scull_qset *next;
};

struct opdracht4_device_data
{
	struct scull_qset *data;
	int quantum;
	int qset;
	unsigned long size;
	struct cdev cdev;
};

static int dev_major = 0;
int scull_quantum = QUANTUM_NUM;
int scull_qset = QSET_SIZE;
static struct class *opdracht4_class = NULL;
static struct opdracht4_device_data opdracht4_data[MAX_DEV];

int opdracht4_trim_data(struct opdracht4_device_data *dev)
{
	struct scull_qset *next, *dptr;
	int qset = dev->qset;
	int i;

	for (dptr = dev->data; dptr; dptr = next)
	{
		if (dptr->data)
		{
			for (i = 0; i < qset; i++)
				kfree(dptr->data[i]);
			kfree(dptr->data);
			dptr->data = NULL;
		}
		next = dptr->next;
		kfree(dptr);
	}
	dev->size = 0;
	dev->quantum = scull_quantum;
	dev->qset = scull_qset;
	dev->data = NULL;
	return 0;
}

static int device_opened = 0;

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

	for (i = 0; i < MAX_DEV; i++)
	{
		cdev_init(&opdracht4_data[i].cdev, &opdracht4_fops);
		opdracht4_data[i].cdev.owner = THIS_MODULE;

		cdev_add(&opdracht4_data[i].cdev, MKDEV(dev_major, i), 1);

		device_create(opdracht4_class, NULL, MKDEV(dev_major, i), NULL, "opdracht4-%d", i);
	}

	return 0;
}

static void opdracht4_exit(void)
{
	int i;

	printk("static void opdracht4_exit(void) called.\n");

	for (i = 0; i < MAX_DEV; i++)
	{
		device_destroy(opdracht4_class, MKDEV(dev_major, i));
		cdev_del(&opdracht4_data[i].cdev);
	}

	class_unregister(opdracht4_class);
	class_destroy(opdracht4_class);

	unregister_chrdev_region(MKDEV(dev_major, 0), MINORMASK);
}

static int opdracht4_open(struct inode *inode, struct file *file)
{
	struct opdracht4_device_data *dev;

	printk("static int opdracht4_open(struct inode *inode, struct file *file) called.\n");
	device_opened++;
	printk("Device opened %d times.\n", device_opened);

	dev = container_of(inode->i_cdev, struct opdracht4_device_data, cdev);
	file->private_data = dev;

	/* trim to 0 the length of the device if open was write-only */
	if ((file->f_flags & O_ACCMODE) == O_WRONLY)
	{
		opdracht4_trim_data(dev);
	}

	return 0;
}

static int opdracht4_release(struct inode *inode, struct file *file)
{
	printk("static int opdracht4_release(struct inode *inode, struct file *file) called.\n");
	return 0;
}

struct scull_qset *opdracht4_follow_data(struct opdracht4_device_data *dev, int n)
{
	struct scull_qset *qs = dev->data;

	if (!qs)
	{
		qs = dev->data = kmalloc(sizeof(struct scull_qset), GFP_KERNEL);
		if (qs == NULL)
			return NULL;
		memset(qs, 0, sizeof(struct scull_qset));
	}

	while (n--)
	{
		if (!qs->next)
		{
			qs->next = kmalloc(sizeof(struct scull_qset), GFP_KERNEL);
			if (qs->next == NULL)
				return NULL;
			memset(qs->next, 0, sizeof(struct scull_qset));
		}
		qs = qs->next;
		continue;
	}
	return qs;
}

static long opdracht4_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	printk("static long opdracht4_ioctl(struct file *file, unsigned int cmd, unsigned long arg) called.\n");
	return 0;
}

static ssize_t opdracht4_read(struct file *file, char __user *buf, size_t count, loff_t *offset)
{
	struct opdracht4_device_data *dev = file->private_data;
	struct scull_qset *dptr;
	int quantum = dev->quantum, qset = dev->qset;
	int itemsize = quantum * qset;
	int item, s_pos, q_pos, rest;
	ssize_t retval = 0;

	printk("static ssize_t opdracht4_read(struct file *file, char __user *buf, size_t count, loff_t *offset) called with:\n");
	printk("size_t count = %ld, loff_t *offset = %lld.\n", count, *offset);

	printk("Reading device: %d\n", MINOR(file->f_path.dentry->d_inode->i_rdev));

	if (*offset >= dev->size)
		goto out;
	if (*offset + count > dev->size)
		count = dev->size - *offset;

	item = (long)*offset / itemsize;
	rest = (long)*offset % itemsize;
	s_pos = rest / quantum;
	q_pos = rest % quantum;

	dptr = opdracht4_follow_data(dev, item);

	if (dptr == NULL || !dptr->data || !dptr->data[s_pos])
		goto out;

	if (count > quantum - q_pos)
		count = quantum - q_pos;

	if (copy_to_user(buf, dptr->data[s_pos] + q_pos, count))
	{
		retval = -EFAULT;
		goto out;
	}
	*offset += count;
	retval = count;

out:
	return retval;
}

static ssize_t opdracht4_write(struct file *file, const char __user *buf, size_t count, loff_t *offset)
{
	struct opdracht4_device_data *dev = file->private_data;
	struct scull_qset *dptr;
	int quantum = dev->quantum, qset = dev->qset;
	int itemsize = quantum * qset;
	int item, s_pos, q_pos, rest;
	ssize_t retval = -ENOMEM;

	printk("static ssize_t opdracht4_write(struct file *file, const char __user *buf, size_t count, loff_t *offset) called.\n");
	printk("size_t count = %ld, loff_t *offset = %lld.\n", count, *offset);

	printk("Writing device: %d\n", MINOR(file->f_path.dentry->d_inode->i_rdev));

	item = (long)*offset / itemsize;
	rest = (long)*offset % itemsize;
	s_pos = rest / quantum;
	q_pos = rest % quantum;

	dptr = opdracht4_follow_data(dev, item);
	if (dptr == NULL)
		goto out;
	if (!dptr->data)
	{
		dptr->data = kmalloc(qset * sizeof(char *), GFP_KERNEL);
		if (!dptr->data)
			goto out;
		memset(dptr->data, 0, qset * sizeof(char *));
	}
	if (!dptr->data[s_pos])
	{
		dptr->data[s_pos] = kmalloc(quantum, GFP_KERNEL);
		if (!dptr->data[s_pos])
			goto out;
	}

	if (count > quantum - q_pos)
		count = quantum - q_pos;

	if (copy_from_user(dptr->data[s_pos] + q_pos, buf, count))
	{
		retval = -EFAULT;
		goto out;
	}
	*offset += count;
	retval = count;

	if (dev->size < *offset)
		dev->size = *offset;
out:
	return retval;
}

static loff_t opdracht4_llseek(struct file *filp, loff_t off, int whence)
{
	struct opdracht4_device_data *dev = filp->private_data;
	loff_t newpos;

	printk("static loff_t opdracht4_llseek(struct file *filp, loff_t off, int whence) called.\n");
	printk("loff_t off = 0x%08llX, int whence = %d.\n", off, whence);

	switch (whence)
	{
	case 0: /* SEEK_SET */
		newpos = off;
		break;
	case 1: /* SEEK_CUR */
		newpos = filp->f_pos + off;
		break;
	case 2: /* SEEK_END */
		newpos = dev->size + off;
		break;
	default:
		return -EINVAL;
	}
	if (newpos < 0)
		return -EINVAL;
	filp->f_pos = newpos;
	return newpos;
}

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Max te Lintelo <max.te.lintelo@gmail.com>");

module_init(opdracht4_init);
module_exit(opdracht4_exit);
