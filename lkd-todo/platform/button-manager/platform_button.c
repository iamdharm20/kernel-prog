/*
 * Copyright (C) 2018  Harman Connected services.
 * Author: Dharmender, sharma <Dharmender.Sharma4@harman.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/version.h>
#include <linux/reboot.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/delay.h>        /* for msleep() */
#include <linux/timer.h>        /* for timers apis */
#include <linux/of_gpio.h>

static unsigned int irqNumber1;
static bool ledOn1;
static bool reset_flag1;

static unsigned int irqNumber2;
static bool ledOn2;
static bool reset_flag2;

struct artik_switch {
	int gpioButton1, gpioLED1, gpioButton2, gpioLED2;
};

static unsigned long flags;
unsigned long IRQflags = IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING | IRQF_SHARED;;

static struct timer_list long_press_timer1;
static struct timer_list long_press_timer2;

static DEFINE_SPINLOCK(data_lock);

void long_press_timer_callback2(unsigned long data)
{
	spin_lock(&data_lock);
	if (reset_flag1 && reset_flag2)
		emergency_restart();
	spin_unlock(&data_lock);
}

static irqreturn_t button2_irq_handler(int irq, void *dev_id)
{
	uint8_t value;
	struct artik_switch *artik_switch = dev_id;

	local_irq_save(flags);
	value = gpio_get_value(artik_switch->gpioButton2);
	if (value == 0) {
		reset_flag2 = true;
		/* Invert the LED state on each button press */
		ledOn2 = !ledOn2;
		gpio_set_value(artik_switch->gpioLED2, ledOn2);

		/* Timer Started on BUtton press for time 4 seconds */
		mod_timer(&long_press_timer2, jiffies + msecs_to_jiffies(4000));
	} else if (value) {
		reset_flag2 = false;
		if (timer_pending(&long_press_timer2))
			del_timer(&long_press_timer2);
	}
	local_irq_restore(flags);
	return IRQ_HANDLED;
}

static int gpioled2_setup(struct device *dev)
{
	int ret = 0;
	struct platform_device *pdev = to_platform_device(dev);
	struct artik_switch *artik_switch = dev_get_drvdata(dev);
	
	/* Is the GPIO a valid GPIO number */
	if (!gpio_is_valid(artik_switch->gpioLED2)) {
		dev_err(&pdev->dev, "gpio is invalid for led2\n");
		return -ENODEV;
	}

	/* Going to set up the LED */
	ledOn2 = true;

	/* GPIOs must be allocated before use */
	ret = gpio_request(artik_switch->gpioLED2, "sysfs");
	if (ret < 0) {
		dev_err(&pdev->dev, "gpioLED1 %d request failed with error : %d\n",
				artik_switch->gpioLED2, ret);
		return ret;
	}

	ret = gpio_direction_output(artik_switch->gpioLED2, ledOn2);
	if (ret < 0) {
		dev_err(&pdev->dev, "gpioLED1 %d failed to set in output mode with error : %d\n",
				artik_switch->gpioLED2, ret);
		goto gpioLED2_free;
	}

	/* GPIO subsystem create a sysfs entry under /sys/class/gpio */
	ret = gpio_export(artik_switch->gpioLED2, false);
	if (ret < 0) {
		dev_err(&pdev->dev, "gpioLED1 %d export failed with error : %d\n",
				artik_switch->gpioLED2, ret);
		goto gpioLED2_free;
	}
	
	return ret;

gpioLED2_free:
	gpio_free(artik_switch->gpioLED2);
	return ret;

}

static int  button2_interrupt_config(struct device *dev)
{
	int ret = 0;
	struct platform_device *pdev = to_platform_device(dev);
	struct artik_switch *artik_switch = dev_get_drvdata(dev);

	/* the bool argument prevents the direction from being changed */
	ret = gpio_request(artik_switch->gpioButton2, "sysfs");
	if (ret < 0) {
		dev_err(&pdev->dev, "gpiobutton %d request failed with error : %d\n",
				artik_switch->gpioButton2, ret);
		gpio_free(artik_switch->gpioLED2);
		return ret;
	}

	ret = gpio_direction_input(artik_switch->gpioButton2);
	if (ret < 0) {
		dev_err(&pdev->dev, "gpioButton1 %d failed to set in input mode : %d\n",
				artik_switch->gpioButton2, ret);
		goto gpioButton2_free;
	}
	ret = gpio_export(artik_switch->gpioButton2, false);
	if (ret < 0) {
		dev_err(&pdev->dev, "gpioButton1 %d failed to export with error : %d\n",
				artik_switch->gpioButton2, ret);
		goto gpioButton2_free;
	}

	/* Debounce the button with a delay of 200ms */
	gpio_set_debounce(artik_switch->gpioButton2, 200);

	irqNumber2 = gpio_to_irq(artik_switch->gpioButton2);
	ret = request_irq(irqNumber2, (irq_handler_t) button2_irq_handler,
			IRQflags, "button2_handler", artik_switch);
	if (ret != 0) {
		dev_err(&pdev->dev, "Irq Request failure : %d\n", ret);
		goto gpioButton2_free;
	}
	
	return ret;

gpioButton2_free:
	gpio_unexport(artik_switch->gpioLED2);
	gpio_free(artik_switch->gpioLED2);
	gpio_free(artik_switch->gpioButton2);
	return ret;

}

void long_press_timer_callback1(unsigned long data)
{
	spin_lock(&data_lock);
	if (reset_flag1 && reset_flag2)
		emergency_restart();
	spin_unlock(&data_lock);
}

static irqreturn_t button1_irq_handler(int irq, void *dev_id)
{
	uint8_t value;
	struct artik_switch *artik_switch = dev_id;

	local_irq_save(flags);

	value = gpio_get_value(artik_switch->gpioButton1);
	if (value == 0) {
		reset_flag1 = true;
		ledOn1 = !ledOn1;
		gpio_set_value(artik_switch->gpioLED1, ledOn1);
		mod_timer(&long_press_timer1, jiffies + msecs_to_jiffies(4000));
	} else if (value) {
		reset_flag1 = false;
		if (timer_pending(&long_press_timer1))
			del_timer(&long_press_timer1);
	}

	local_irq_restore(flags);
	return IRQ_HANDLED;
}

static int gpioled1_setup(struct device *dev)
{

	int ret = 0;
	struct platform_device *pdev = to_platform_device(dev);
	struct artik_switch *artik_switch = dev_get_drvdata(dev);
	
	/* Is the GPIO a valid GPIO number */
	if (!gpio_is_valid(artik_switch->gpioLED1)) {
		dev_err(&pdev->dev, "gpio is invalid for led1\n");
		return -ENODEV;
	}

	/* Going to set up the LED */
	ledOn1 = true;

	/* GPIOs must be allocated before use */
	ret = gpio_request(artik_switch->gpioLED1, "sysfs");
	if (ret < 0) {
		dev_err(&pdev->dev, "gpioLED1 %d request failed with error : %d\n",
				artik_switch->gpioLED1, ret);
		return ret;
	}

	ret = gpio_direction_output(artik_switch->gpioLED1, ledOn1);
	if (ret < 0) {
		dev_err(&pdev->dev, "gpioLED1 %d failed to set in output mode with error : %d\n",
				artik_switch->gpioLED1, ret);
		goto gpioLED1_free;
	}

	/* GPIO subsystem create a sysfs entry under /sys/class/gpio */
	ret = gpio_export(artik_switch->gpioLED1, false);
	if (ret < 0) {
		dev_err(&pdev->dev, "gpioLED1 %d export failed with error : %d\n",
				artik_switch->gpioLED1, ret);
		goto gpioLED1_free;
	}
	
	return ret;

gpioLED1_free:
	gpio_free(artik_switch->gpioLED1);
	return ret;

}

static int  button1_interrupt_config(struct device *dev)
{

	int ret = 0;
	struct platform_device *pdev = to_platform_device(dev);
	struct artik_switch *artik_switch = dev_get_drvdata(dev);

	ret = gpio_request(artik_switch->gpioButton1, "sysfs");
	if (ret < 0) {
		dev_err(&pdev->dev, "gpiobutton %d request failed with error : %d\n",
				artik_switch->gpioButton1, ret);
		gpio_free(artik_switch->gpioLED1);
		return ret;
	}
	ret = gpio_direction_input(artik_switch->gpioButton1);
	if (ret < 0) {
		dev_err(&pdev->dev, "gpioButton1 %d failed to set in input mode : %d\n",
				artik_switch->gpioButton1, ret);
		goto gpioButton1_free;
	}

	ret = gpio_export(artik_switch->gpioButton1, false);
	if (ret < 0) {
		dev_err(&pdev->dev, "gpioButton1 %d failed to export with error : %d\n",
				artik_switch->gpioButton1, ret);
		goto gpioButton1_free;
	}

	/* Debounce the button with a delay of 200ms */
	gpio_set_debounce(artik_switch->gpioButton1, 200);

	/* get the interrupt number of this button */
	irqNumber1 = gpio_to_irq(artik_switch->gpioButton1);
	ret = request_irq(irqNumber1, (irq_handler_t) button1_irq_handler,
			IRQflags, "button1_handler", artik_switch);
	if (ret != 0) {
		dev_err(&pdev->dev, "irq Request failure : %d\n", ret);
		goto gpioButton1_free;
	}
	
	return ret;

gpioButton1_free:
	gpio_unexport(artik_switch->gpioLED1);
	gpio_free(artik_switch->gpioLED1);
	gpio_free(artik_switch->gpioButton1);
	return ret;

}

static int gpio_shortlong_gpio_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct artik_switch *artik_switch;
	struct device_node *np = pdev->dev.of_node;

	if (!np)
		return -ENODEV;

	dev_info(&pdev->dev, "probed\n");

	artik_switch = devm_kzalloc(&pdev->dev,
				sizeof(*artik_switch), GFP_KERNEL);
	if (!artik_switch)
		return -ENOMEM;

	artik_switch->gpioButton1 = of_get_named_gpio(np, "button1-gpio", 0);
	if (!gpio_is_valid(artik_switch->gpioButton1)) {
		dev_err(&pdev->dev, "invalid gpio (data): %d\n",
				artik_switch->gpioButton1);
		return -ENODEV;
	}

	artik_switch->gpioLED1 = of_get_named_gpio(np, "led1-gpio", 0);
	if (!gpio_is_valid(artik_switch->gpioLED1)) {
		dev_err(&pdev->dev, "invalid gpio (data): %d\n",
				artik_switch->gpioLED1);
		return -ENODEV;
	}

	artik_switch->gpioButton2 = of_get_named_gpio(np, "button2-gpio", 0);
	if (!gpio_is_valid(artik_switch->gpioButton2)) {
		dev_err(&pdev->dev, "invalid gpio (data): %d\n",
				artik_switch->gpioButton2);
		return -ENODEV;
	}

	artik_switch->gpioLED2 = of_get_named_gpio(np, "led2-gpio", 0);
	if (!gpio_is_valid(artik_switch->gpioLED2)) {
		dev_err(&pdev->dev, "invalid gpio (data): %d\n",
				artik_switch->gpioLED2);
		return -ENODEV;
	}
	dev_info(&pdev->dev," gpioButton1:%d gpioLED1:%d gpioButton2:%d gpioLED2:%d\n",
			artik_switch->gpioButton1, artik_switch->gpioLED1,
			artik_switch->gpioButton2, artik_switch->gpioLED2);

	platform_set_drvdata(pdev, artik_switch);

	ret = gpioled1_setup(&pdev->dev);
	if (ret < 0) {
		dev_err(&pdev->dev,"gpioled_setup failed: %d\n", ret);
		return ret;
	}
	ret = button1_interrupt_config(&pdev->dev);
	if (ret != 0) {
		dev_err(&pdev->dev,"button interrupt configuration failed : %d\n", ret);
		return ret;
	}
	setup_timer(&long_press_timer1, long_press_timer_callback1, 0);

	ret = gpioled2_setup(&pdev->dev);
	if (ret < 0) {
		dev_err(&pdev->dev,"gpioled_setup failed: %d\n", ret);
		return ret;
	}
	ret = button2_interrupt_config(&pdev->dev);
	if (ret != 0) {
		dev_err(&pdev->dev,"button interrupt configuration failed : %d\n", ret);
		return ret;
	}
	setup_timer(&long_press_timer2, long_press_timer_callback2, 0);

	return ret;
}

static int gpio_shortlong_gpio_remove(struct platform_device *pdev)
{

	struct artik_switch *artik_switch;

	artik_switch = platform_get_drvdata(pdev);

	free_irq(irqNumber2, NULL);

	/* Turn the LED off, makes it clear the device was unloaded */
	gpio_set_value(artik_switch->gpioLED2, 0);
	gpio_unexport(artik_switch->gpioLED2);
	gpio_free(artik_switch->gpioLED2);
	gpio_unexport(artik_switch->gpioButton2);
	gpio_free(artik_switch->gpioButton2);

	free_irq(irqNumber1, NULL);
	gpio_set_value(artik_switch->gpioLED1, 0);
	gpio_unexport(artik_switch->gpioLED1);
	gpio_free(artik_switch->gpioLED1);

	gpio_unexport(artik_switch->gpioButton1);
	gpio_free(artik_switch->gpioButton1);

	dev_info(&pdev->dev,"removed\n");
	return 0;
}

static const struct of_device_id gpio_of_ids[] = {
	{ .compatible = "switch40x-gpios" },
	{ }
};
MODULE_DEVICE_TABLE(of, gpio_of_ids);

static struct platform_driver gpio_shortlong_gpio_driver = {
	.driver = {
		.name = "gpio-button-manager",
		.of_match_table = gpio_of_ids,
	},
	.probe  = gpio_shortlong_gpio_probe,
	.remove = gpio_shortlong_gpio_remove,
};

static int __init gpio_init(void)
{
	return platform_driver_register(&gpio_shortlong_gpio_driver);
}

static void __exit gpio_exit(void)
{
	platform_driver_unregister(&gpio_shortlong_gpio_driver);
}

module_init(gpio_init);
module_exit(gpio_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Dharmender Sharma");
MODULE_DESCRIPTION("simpli-fi-dev-gpio driver");
MODULE_VERSION("1.0");
