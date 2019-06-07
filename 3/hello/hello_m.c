#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h> // task_struct
MODULE_LICENSE( "GPL v2" );
MODULE_AUTHOR("iu7");
MODULE_DESCRIPTION("Simple loadable kernel module");

static int __init hello_init( void ) 
{
    printk(KERN_INFO "Hello, world!\n");
    printk("The process id is %d\n", current->pid);
    printk("The process name is %s\n", current->comm);
    return 0;
}

static void __exit hello_exit( void ) 
{
    printk(KERN_INFO "Goodbye, world!\n" );
    printk("The process id is %d\n", current->pid);
}
module_init( hello_init );
module_exit( hello_exit );
