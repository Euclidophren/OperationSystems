#include <linux/init.h>
#include <linux/module.h>
#include "md.h"

MODULE_LICENSE( "GPL" );

static int __init md_init( void ) 
{
    printk("+ module md3 start\n" );
    printk("+ message exported: %s\n", md1_data);
    printk("+ message returned: %s\n", md1_proc());
    return -1;
}

static void __exit md_exit( void ) 
{
    printk("- module md3 unload\n" );
}
module_init( md_init );
module_exit( md_exit );
