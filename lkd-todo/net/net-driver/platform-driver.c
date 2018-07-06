#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/platform_device.h>
 
 
static int sample_module_drv_probe(struct device *dev)
{
	printk(KERN_DEBUG "probe sample module device\n");
	return 0;
}
 
static int sample_module_drv_remove(struct device *dev)
{
	printk(KERN_DEBUG "remove sample module device\n");
	return 0;
}
 
static struct device_driver sample_module_driver = {
	.name           = "sample_module",
	.bus            = &platform_bus_type,
	.probe          = sample_module_drv_probe,
	.remove         = sample_module_drv_remove,
};
 
static int __init sample_module_init(void)
{
	printk(KERN_DEBUG "init sample module device\n");
	return driver_register(&sample_module_driver);
}
 
static void __exit sample_module_cleanup(void)
{
	printk(KERN_DEBUG "cleanup sample module device\n");
	driver_unregister(&sample_module_driver);
}
 
module_init(sample_module_init);
module_exit(sample_module_cleanup);

MODULE_DESCRIPTION("sample module");
MODULE_LICENSE("GPL");
