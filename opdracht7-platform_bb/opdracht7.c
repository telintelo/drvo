#include <linux/module.h>   
#include <linux/string.h>    
#include <linux/fs.h>      
#include <asm/uaccess.h>
#include <linux/init.h>
#include <linux/cdev.h>

#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/sched.h>
#include <linux/interrupt.h>

#include <linux/list.h>
#include <linux/irq.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/spi/spi.h>
#include <linux/io.h>

#define MY_DEV_NAME "opdracht7"

static int opdracht7_probe(struct platform_device *pdev)
{
	return 0;
}

static int opdracht7_remove(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver opdracht7_driver= {
	.driver		= {
		.name	= MY_DEV_NAME,
		.owner	= THIS_MODULE,
	},
	.probe		= opdracht7_probe,
	.remove		= opdracht7_remove,
};

static int __init opdracht7_init(void)
{
	printk("static int __init opdracht7_init(void)\n");
	return platform_driver_register(&opdracht7_driver);
	// return platform_driver_probe(&opdracht7_driver, opdracht7_probe);
}

static void __exit opdracht7_exit(void)
{
	printk("static void __exit opdracht7_exit(void)\n");
	platform_driver_unregister(&opdracht7_driver);
}

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Max te Lintelo <max.te.lintelo@gmail.com>");

module_init(opdracht7_init);
module_exit(opdracht7_exit);
