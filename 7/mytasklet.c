#include <linux/module.h> 
#include <linux/kernel.h> 
#include <linux/init.h> 
#include <linux/interrupt.h>
#include <linux/ktime.h>

#define SHARED_IRQ 1

MODULE_LICENSE("GPL");

static int irq = SHARED_IRQ, my_dev_id, irq_counter = 0;

char tasklet_data[] = "tasklet was called";

void tasklet_function(unsigned long data);

DECLARE_TASKLET(my_tasklet, tasklet_function, (unsigned long)&tasklet_data);

void tasklet_function(unsigned long data) 
{
  irq_counter++;
  printk(KERN_INFO "TASKLET count: %d, data: %s, time: %lld ns", irq_counter, my_tasklet.data, ktime_get_ns());
  return;
}	

static irqreturn_t my_interrupt(int irq, void *dev_id) 
{
  if (irq == SHARED_IRQ)
  {
    tasklet_schedule(&my_tasklet);
    return IRQ_HANDLED;      
  }
  else
    return IRQ_NONE;
}

static int __init my_tasklet_init(void) 
{
  if(request_irq(irq, my_interrupt, IRQF_SHARED, "my_interrupt", &my_dev_id))
    return -1;
  printk(KERN_INFO "Successfully loading on IRQ %d\n", irq);
  printk(KERN_INFO "Module is now loaded.\n");
  return 0;
}

static void __exit my_tasklet_exit(void) 
{
  tasklet_kill(&my_tasklet);
  free_irq(irq, &my_dev_id);
  printk(KERN_INFO "Successfully unloading\n");
  printk(KERN_INFO "Module is now unloaded.\n");
  return;
}

module_init(my_tasklet_init);
module_exit(my_tasklet_exit);
