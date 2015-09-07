#include <linux/init.h>
#include <linux/module.h>

// Special kernel macro to indicate license
MODULE_LICENSE("Dual BSD/GPL");

static int hello_init(void)
{
	printk(KERN_ALERT "Hello world!\n");
}

static void hello_exit(void)
{
	printk(KERN_ALERT "Goodbye world!\n");
}

// Special kernel macros to indicate init and exit roles
module_init(hello_init);
module_exit(hello_exit);
