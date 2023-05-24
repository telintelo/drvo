#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/slab.h>

struct my_device_platform_data {
	int reset_gpio;
	int power_on_gpio;
	void (*power_on)(struct my_device_platform_data* ppdata);
	void (*power_off)(struct my_device_platform_data* ppdata);
	void (*reset)(struct my_device_platform_data* pdata);
};

static void my_device_power_on(struct my_device_platform_data* pdata)
{
    printk(KERN_ALERT " %s\n", __FUNCTION__);
}

static void my_device_power_off(struct my_device_platform_data* pdata)
{
    printk(KERN_ALERT " %s\n", __FUNCTION__);
}

static void my_device_reset(struct my_device_platform_data* pdata)
{
    printk(KERN_ALERT " %s\n", __FUNCTION__);
}

static struct my_device_platform_data my_device_pdata = {
    .reset_gpio = 100,
    .power_on_gpio = 101,
    .power_on = my_device_power_on,
    .power_off = my_device_power_off,
    .reset = my_device_reset
};

static struct platform_device my_device = {
    .name = "my-platform-device",
    .id = PLATFORM_DEVID_NONE,
    .dev.platform_data = &my_device_pdata
};

void __init my_device_init_pdata(void)
{
    printk(KERN_ALERT " %s\n", __FUNCTION__);
    platform_device_register(&my_device);
}

struct my_driver_data {
    int data1;
    int data2;
};

static int my_device_pm_suspend(struct device *dev)
{
    struct my_driver_data* driver_data = dev_get_drvdata(dev);
    printk(KERN_ALERT " %s\n", __FUNCTION__);
    return 0;
}

static int my_device_pm_resume(struct device *dev)
{
    struct my_driver_data* driver_data = dev_get_drvdata(dev);
    printk(KERN_ALERT " %s\n", __FUNCTION__);
    return 0;
}

static int my_driver_probe(struct platform_device *pdev)
{
    struct my_device_platform_data *my_device_pdata;
    struct my_driver_data* driver_data;

    printk(KERN_ALERT " %s\n", __FUNCTION__);

    my_device_pdata = dev_get_platdata(&pdev->dev);

    if (my_device_pdata->power_on) {
        my_device_pdata->power_on(my_device_pdata);
    }

    mdelay(5);

    if (my_device_pdata->reset) {
        my_device_pdata->reset(my_device_pdata);
    }

    driver_data = kzalloc(sizeof(struct my_driver_data), GFP_KERNEL);
    if (!driver_data)
        return -ENOMEM;

    platform_set_drvdata(pdev, driver_data);

    return 0;
}

static int my_driver_remove(struct platform_device *pdev)
{
    struct my_device_platform_data *my_device_pdata = dev_get_platdata(&pdev->dev);;
    struct my_driver_data *driver_data = platform_get_drvdata(pdev);

    printk(KERN_ALERT " %s\n", __FUNCTION__);

    if (my_device_pdata->power_off) {
        my_device_pdata->power_off(my_device_pdata);
    }

    return 0;
}

static const struct dev_pm_ops my_device_pm_ops = {
    .suspend = my_device_pm_suspend,
    .resume = my_device_pm_resume,
};

static struct platform_driver my_driver = {
    .probe = my_driver_probe,
    .remove = my_driver_remove,
    .driver = {
        .name = "my-platform-device",
        .owner = THIS_MODULE,
        .pm	= &my_device_pm_ops,
    },
};

static int __init my_driver_init_module(void)
{
    int ret;
    pr_info(" %s\n", __FUNCTION__);

    my_device_init_pdata();

    ret = platform_driver_probe(&my_driver, my_driver_probe);

    return ret;
}

static void __exit my_driver_cleanup_module(void)
{
    pr_info(" %s\n", __FUNCTION__);
    platform_driver_unregister(&my_driver);
}

module_init(my_driver_init_module);
module_exit(my_driver_cleanup_module);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Max te Lintelo <max.te.lintelo@gmail.com>");
