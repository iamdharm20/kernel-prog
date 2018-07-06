#include <linux/module.h>
#include <linux/platform_device.h>

MODULE_AUTHOR("Brook");
MODULE_DESCRIPTION("Kernel module for demo");
MODULE_LICENSE("GPL");

#define DEVNAME "ptest"

#define DYN_ALLOC 1

static struct platform_device *ptest_device;

static int ptest_probe(struct platform_device *pdev)
{
    pr_info("%s(#%d)\n", __func__, __LINE__);
    return 0;
}

static int ptest_remove(struct platform_device *pdev)
{
    pr_info("%s(#%d)\n", __func__, __LINE__);
    return 0;
}

static struct platform_driver ptest_driver = {
    .driver = {
        .name  = DEVNAME,
        .owner = THIS_MODULE,
    },
    .probe  = ptest_probe,
    .remove = ptest_remove,
};

static int __init ptest_init(void)
{
    int err;
    pr_info("%s(#%d)\n", __func__, __LINE__);

    /* using platform_device_alloc() + platform_device_add() 
     * instead of platform_device_register() to avoid the OOPS, 
     *     "Device 'ptest.0' does not have a release() function,
     *      it is broken and must be fixed."
     */
    ptest_device = platform_device_alloc(DEVNAME, 0);
    if (!ptest_device) {
        pr_err("%s(#%d): platform_device_alloc fail\n",
               __func__, __LINE__);
        return -ENOMEM;
    }

    err = platform_device_add(ptest_device);
    if (err) {
        pr_err("%s(#%d): platform_device_add failed\n",
               __func__, __LINE__);
        goto dev_add_failed;
    }

    err = platform_driver_register(&ptest_driver);
    if (err) {
        dev_err(&(ptest_device->dev), "%s(#%d): platform_driver_register fail(%d)\n",
                __func__, __LINE__, err);
        goto dev_reg_failed;
    }
    return err;

dev_add_failed:
    platform_device_put(ptest_device);
dev_reg_failed:
    platform_device_unregister(ptest_device);

    return err;
}
module_init(ptest_init);

static void __exit ptest_exit(void)
{
    dev_info(&(ptest_device->dev), "%s(#%d)\n", __func__, __LINE__);
    platform_device_unregister(ptest_device);
    platform_driver_unregister(&ptest_driver);
}
module_exit(ptest_exit);
