#include <linux/module.h>
#include <linux/platform_device.h>


#define DEVNAME "ptest"

#define DYN_ALLOC 1

static struct platform_device ptest_device = {
    .name = DEVNAME,
};

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

    err = platform_device_register(&ptest_device);
    if (err) {
        pr_err("%s(#%d): platform_device_register failed(%d)\n",
                __func__, __LINE__, err);
        return err;
    }

    err = platform_driver_register(&ptest_driver);
    if (err) {
        dev_err(&(ptest_device.dev), "%s(#%d): platform_driver_register fail(%d)\n",
                __func__, __LINE__, err);
        goto dev_reg_failed;
    }
    return err;

dev_reg_failed:
    platform_device_unregister(&ptest_device);

    return err;
}

static void __exit ptest_exit(void)
{
    dev_info(&(ptest_device.dev), "%s(#%d)\n", __func__, __LINE__);
    platform_device_unregister(&ptest_device);
    platform_driver_unregister(&ptest_driver);
}

module_init(ptest_init);
module_exit(ptest_exit);

MODULE_AUTHOR("Brook");
MODULE_DESCRIPTION("Kernel module for demo");
MODULE_LICENSE("GPL");
