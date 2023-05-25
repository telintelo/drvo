#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/mod_devicetable.h>
#include <linux/of.h>
#include <linux/ioport.h>

#define MY_DEV_NAME "drvo-final"

static int opdracht7_probe(struct platform_device *pdev)
{
	printk("static int opdracht7_probe(struct platform_device *pdev)\n");
	return 0;
}

static int opdracht7_remove(struct platform_device *pdev)
{
	printk("static int opdracht7_remove(struct platform_device *pdev)\n");
	return 0;
}

static const struct of_device_id g_ids[] = {
	{ .compatible = MY_DEV_NAME, },
	{ }
};

static struct platform_driver opdracht7_driver= {
	.driver		= {
		.name	= MY_DEV_NAME,
		.owner	= THIS_MODULE,
		.of_match_table = of_match_ptr(g_ids),
	},
	.probe		= opdracht7_probe,
	.remove		= opdracht7_remove,
};

static int __init opdracht7_init(void)
{
	printk("static int __init opdracht7_init(void)\n");
	return platform_driver_register(&opdracht7_driver);
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
