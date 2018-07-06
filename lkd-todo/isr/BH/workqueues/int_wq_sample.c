#include <linux/kernel.h>    /* We're doing kernel work */
#include <linux/module.h>    /* Specifically, a module */
#include <linux/sched.h>
#include <linux/workqueue.h>
#include <linux/interrupt.h>    /* We want an interrupt */
#include <asm/io.h>
#include <linux/slab.h>
 
#define MY_WORK_QUEUE_NAME "wq_test"
#define printd() \
        printk(KERN_ALERT "workqueue_test: %s: %d: caller: %pS\n", __func__, __LINE__, __builtin_return_address(0));
 
static struct workqueue_struct *my_workqueue;
struct work_data {
        struct work_struct work;
        int data;
	unsigned char status;
	unsigned char scancode;
};
 
/* 
 * This will get called by the kernel as soon as it's safe
 * to do everything normally allowed by kernel modules.
 */
//static void got_char(void *scancode)
static void work_handler(struct work_struct *work)
{
        struct work_data * data = (struct work_data *)work;
        printd();
	//printk("Scan Code %x %s.\n",(int) * ( (char *)data->scancode ) & 0x7F, *( (char *)data->scancode ) & 0x80 ? "Released" : "Pressed");
	printk(KERN_INFO "Scan Code %x %s.\n", data->scancode & 0x7F, data->scancode & 0x80 ? "Released" : "Pressed");
        kfree(data);
}
 
/*http://elixir.free-electrons.com/linux/latest/source/drivers/spi/spi-mpc52xx.c#L335 
 * This function services keyboard interrupts. It reads the relevant
 * information from the keyboard and then puts the non time critical
 * part into the work queue. This will be run when the kernel considers it safe.
 */
static irqreturn_t irq_handler(int irq, void *dev_id) 
{

	struct work_data *data = kmalloc(sizeof(struct work_data), GFP_KERNEL);

	INIT_WORK(&data->work, work_handler);

	/* Read keyboard status : unsigned char inb(unsigned short int port); */
	data->status = inb(0x64);
	data->scancode = inb(0x60);
 
	queue_work(my_workqueue, &data->work);

	return IRQ_HANDLED;
}
 
/* Initialize the module - register the IRQ handler */
static int __init keyboard_init( void )
{
	int ret = 0;

	printk("%s : Caller is %pS\n", __func__, __builtin_return_address(0));

	my_workqueue = create_workqueue(MY_WORK_QUEUE_NAME);

	/* 
	* Since the keyboard handler won't co-exist with another handler,
	* such as us, we have to disable it (free its IRQ) before we do
	* anything.  Since we don't know where it is, there's no way to
	* reinstate it later - so the computer will have to be rebooted
	* when we're done.
	*/
	free_irq(1, NULL);
	
	/*
	int request_irq(unsigned int irq, irq_handler_t handler, unsigned long flags, const char *name, void *dev)
		return request_threaded_irq(irq, handler, NULL, flags, name, dev);

	devm_request_irq(struct device *dev, unsigned int irq, irq_handler_t handler, unsigned long irqflags, const char *devname, void *dev_id)

	request_threaded_irq — allocate an interrupt line
	int request_threaded_irq (unsigned int irq, irq_handler_t handler,irq_handler_t thread_fn, unsigned long irqflags, 
					const char *devname, void *dev_id);
	Arguments
	irq - Interrupt line to allocate
	handler - Function to be called when the IRQ occurs. Primary handler for threaded interrupts If NULL and thread_fn != NULL the default 
		primary handler is installed
	thread_fn - Function called from the irq handler thread If NULL, no irq thread is created
	irqflags - Interrupt type flags
	devname - An ascii name for the claiming device
	dev_id - A cookie passed back to the handler function

	*/ 

	/* 
	* Request IRQ 1, the keyboard IRQ, to go to our irq_handler.
	* SA_SHIRQ means we're willing to have othe handlers on this IRQ.
	* SA_INTERRUPT can be used to make the handler into a fast interrupt.
	*/
	ret= request_irq(1,    /* The number of the keyboard IRQ on PCs */
			irq_handler,    /* our handler */
			IRQF_SHARED | IRQF_NO_SUSPEND, "test_keyboard_irq_handler", (void *)(irq_handler));
        if (ret) {
                printk(KERN_INFO "short: can't get assigned irq : 1 \n");
        }
        return ret;

}
 
static void __exit keyboard_cleanup( void )
{
	/*
	* This is only here for completeness. It's totally irrelevant, since
	* we don't have a way to restore the normal keyboard interrupt so the
	* computer is completely useless and has to be rebooted.
	*/
	free_irq(1, NULL);
}

module_init(keyboard_init);
module_exit(keyboard_cleanup);
 
/* some work_queue related functions are just available to GPL licensed Modules */
MODULE_LICENSE("GPL");
