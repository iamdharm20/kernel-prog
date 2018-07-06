#include <linux/interrupt.h>  
#define IRQ_NUM 13  
  
static irqreturn_t hard_irq_handler(int irq, void *dev_id)  
{  
    /* 
     * irq context, limited operations. 
     * do emergency work and quit as soon as possible 
     */  
    return IRQ_WAKE_THREAD;  
}  
  
static irqreturn_t thread_irq_handler(int irq, void *dev_id)  
{  
    /* 
     * do the real work 
     * you can use sleep api, mutex/semaphore api, 
     * you can access user space memory here, 
     */  
    return IRQ_HANDLED;  
}  
  
static int irq_sample(struct device *dev)  
{  
    int irq = IRQ_NUM;  
    int r;  
  
    r = devm_request_threaded_irq(dev, irq, hard_irq_handler,  
            thread_irq_handler,  
            IRQF_TRIGGER_FALLING | IRQF_ONESHOT,  
            "samp",  
            NULL);  
    if (r < 0)  
        return r;  
    // at this point, irq is requested and is enabled.  
    return 0;  
} 
