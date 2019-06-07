#include <linux/module.h> 
#include <linux/kernel.h> 
#include <linux/init.h> 
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/ktime.h>

#define SHARED_IRQ 1

MODULE_LICENSE("GPL");
MODULE_AUTHOR("pokasovaai");

static int irq = SHARED_IRQ, my_dev_id, irq_counter = 0;

struct workqueue_struct *wq;
void work_function(struct work_struct *work);

DECLARE_WORK(hardwork, work_function);

void work_function(struct work_struct *work) 
{
  irq_counter++;
  printk(KERN_INFO "count: %d, interrupt time: %ld ",irq_counter, ktime_get_ns());
  return;
}

static irqreturn_t my_interrupt(int irq, void *dev_id) 
{ 
  if (irq == SHARED_IRQ)
  {
    queue_work(wq, &hardwork);
    return IRQ_HANDLED;    
  }
  else
    return IRQ_NONE;  
}

static int __init my_wokqueue_init(void) 
{
  if(request_irq(irq, my_interrupt, IRQF_SHARED, "mywork", &my_dev_id))
    return -1;
  printk(KERN_INFO "Successfully loading on IRQ %d\n", irq);
  wq = alloc_workqueue("my_queue", WQ_UNBOUND,0);
  if (!wq) 
  {
    free_irq(irq, &my_dev_id);
    return -ENOMEM;
  }
  printk(KERN_INFO "Workqueue created.\n");
  printk(KERN_INFO "Module is now loaded.\n");
  return 0;
}

static void __exit my_wokqueue_exit(void) 
{
  flush_workqueue(wq);
  destroy_workqueue(wq);
  synchronize_irq(irq);
  free_irq(irq, &my_dev_id);
  printk(KERN_INFO "Successfully unloading\n");
  printk(KERN_INFO "Module is now unloaded.\n");
  return;
}

module_init(my_wokqueue_init);
module_exit(my_wokqueue_exit);
