#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/of.h>
#include <linux/regmap.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/device.h>

#include "drvo-final.h"

#define DEBUG

#define DRVO_COMMAND_ID 0xD0
#define DRVO_COMMAND_DATA 0xF7

#define COMPATIBLE_NAME "drvo-final"

static struct class *drvo_class;
static struct device *drvo_dev;

struct drvo_data {
    struct i2c_client *client;
    int8_t temperature;
    uint8_t mode;
    struct device_attribute attr_temperature_async;
    // struct device_attribute attr_temperature_sync;
    struct device_attribute attr_mode;
};

static struct task_struct *poll_thread = NULL; // Add poll_thread member

static ssize_t drvo_show_temperature_async(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct drvo_data *data = dev_get_drvdata(dev);

    DRVO_info("Entered drvo_show_temperature_async(struct device *dev, struct device_attribute *attr, char *buf)\n");

    return snprintf(buf, PAGE_SIZE, "Temperature: %d, mode: %d.\n", data->temperature, data->mode);
}

// static ssize_t drvo_show_temperature_sync(struct device *dev, struct device_attribute *attr, char *buf)
// {
//     struct drvo_data *data = dev_get_drvdata(dev);
//     int8_t temperature;

//     DRVO_info("Entered drvo_show_temperature_sync(struct device *dev, struct device_attribute *attr, char *buf)\n");

//     i2c_smbus_read_i2c_block_data(data->client, DRVO_COMMAND_DATA, sizeof(temperature), &temperature);

//     return snprintf(buf, PAGE_SIZE, "Temperature: %d (sync), mode: %d.\n", temperature, data->mode);
// }

static ssize_t drvo_store_mode(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct drvo_data *data = dev_get_drvdata(dev);
    int val;

    DRVO_info("Entered drvo_store_mode(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)\n");

    if (kstrtoint(buf, 10, &val) == 0) {
        data->mode = val;
        return count;
    }

    return -EINVAL;
}

static int drvo_poll_thread(void *data)
{
    struct drvo_data *drv_data = data;

    DRVO_info("Thread started\n");
    while (!kthread_should_stop()) {
        i2c_smbus_read_i2c_block_data(drv_data->client, DRVO_COMMAND_DATA, sizeof(drv_data->temperature), &(drv_data->temperature));
        msleep(1000);
    }
    DRVO_info("Thread stopped\n");

    return 0;
}

static int drvo_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    struct device_node *np = client->dev.of_node;
    struct drvo_data *data;
    int addr;
    int ret;

    DRVO_info("Entered drvo_probe(struct i2c_client *client, const struct i2c_device_id *id)\n");

    if (!of_device_is_compatible(np, COMPATIBLE_NAME)) {
        DRVO_err("Device is not compatible.\n");
        return -ENODEV;
    }
    if (of_property_read_u32(np, "reg", &addr)) {
        DRVO_err("Failed to read I2C address\n");
        return -EINVAL;
    }
    data = devm_kzalloc(&client->dev, sizeof(struct drvo_data), GFP_KERNEL);
    if (!data)
        return -ENOMEM;
    data->client = client;

    ret = i2c_smbus_read_byte_data(client, DRVO_COMMAND_ID);
    if (ret < 0) {
        DRVO_err("Failed to read chip ID\n");
        return ret;
    }
    if (ret != 0x58) {
        DRVO_err("Invalid chip ID: %02X\n", ret);
        return -ENODEV;
    }

    // Create a kernel thread for polling temperature
    poll_thread = kthread_create(drvo_poll_thread, data, "drvo_poll");
    if (poll_thread) {
        wake_up_process(poll_thread);
    } else {
        DRVO_err("Failed to create polling thread\n");
        return PTR_ERR(poll_thread);
    }
    i2c_set_clientdata(client, data);

    drvo_class = class_create(THIS_MODULE, "drvo-final");
    if (IS_ERR(drvo_class)) {
        DRVO_err("Failed to create sysfs class\n");
        return PTR_ERR(drvo_class);
    }
    drvo_dev = device_create(drvo_class, &client->dev, dev_name(&client->dev), data, "drvo-final");
    if (IS_ERR(drvo_dev)) {
        DRVO_err("Failed to create sysfs device\n");
        class_destroy(drvo_class);
        return PTR_ERR(drvo_dev);
    }

    data->attr_temperature_async.attr.name = "temperature_async";
    data->attr_temperature_async.attr.mode = S_IRUGO;
    data->attr_temperature_async.show = drvo_show_temperature_async;
    ret = device_create_file(drvo_dev, &data->attr_temperature_async);
    if (ret) {
        DRVO_err("Failed to create async temperature file.\n");
        return ret;
    }

    // data->attr_temperature_async.attr.name = "temperature_sync";
    // data->attr_temperature_async.attr.mode = S_IRUGO;
    // data->attr_temperature_async.show = drvo_show_temperature_sync;
    // ret = device_create_file(drvo_dev, &data->attr_temperature_sync);
    // if (ret) {
    //     DRVO_err("Failed to create sync temperature file.\n");
    //     return ret;
    // }

    data->attr_mode.attr.name = "mode";
    data->attr_mode.attr.mode = S_IWUGO;
    data->attr_mode.store = drvo_store_mode;
    ret = device_create_file(drvo_dev, &data->attr_mode);
    if (ret) {
        DRVO_err("Failed to create mode file.\n");
        return ret;
    }

    DRVO_info("Probe successful.\n");

    return 0;
}

static int drvo_remove(struct i2c_client *client)
{
    struct drvo_data *data = i2c_get_clientdata(client);

    DRVO_info("Entered drvo_remove(struct i2c_client *client)\n");

    if (poll_thread) {
        kthread_stop(poll_thread);
        poll_thread = NULL;
    }

    device_remove_file(&client->dev, &data->attr_temperature_async);
    // device_remove_file(&client->dev, &data->attr_temperature_sync);
    device_remove_file(&client->dev, &data->attr_mode);
    device_destroy(drvo_class, dev_name(&client->dev));

    class_destroy(drvo_class);

    i2c_set_clientdata(client, NULL);

    return 0;
}

static const struct i2c_device_id drvo_id[] = {
    {COMPATIBLE_NAME, 0},
    {}
};
MODULE_DEVICE_TABLE(i2c, drvo_id);

static struct of_device_id drvo_of_match[] = {
    {.compatible = COMPATIBLE_NAME},
    {}
};
MODULE_DEVICE_TABLE(of, drvo_of_match);

static struct i2c_driver drvo_driver = {
    .probe = drvo_probe,
    .remove = drvo_remove,
    .id_table = drvo_id,
    .driver = {
        .name = COMPATIBLE_NAME,
        .of_match_table = drvo_of_match,
    },
};

static int __init drvo_init(void)
{
    int ret;

    DRVO_info("Registering I2C driver.\n");
    ret = i2c_add_driver(&drvo_driver);
    if (ret < 0) {
        DRVO_err("Failed to register driver: %d.\n", ret);
        return ret;
    }
    DRVO_info("Successfully registered I2C driver.\n");
    return 0;
}

static void __exit drvo_exit(void)
{
    DRVO_info("Unregistering I2C driver.\n");
    i2c_del_driver(&drvo_driver);
    DRVO_info("Successfully unregistered I2C driver\n");
}

module_init(drvo_init);
module_exit(drvo_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Max te Lintelo <max.te.lintelo@gmail.com>");
MODULE_DESCRIPTION("DRVO Final Driver");
