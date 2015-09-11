#include <linux/init.h>
#include <linux/module.h>
#include <linux/syscalls.h>

// Special kernel macro to indicate license
MODULE_LICENSE("Dual BSD/GPL");

static int hello_init(void)
{
	unsigned long offset;
	unsigned long **sct;
	for (offset = PAGE_OFFSET; offset < ULLONG_MAX; offset += sizeof(void*))
	{
		sct = (unsigned long **) offset;
		if (sct[__NR_close] == (unsigned long *) sys_close)
		{
			printk(KERN_ALERT "Found sys_close\n");
		}
	}
	return 0;
}

static void hello_exit(void)
{
	printk(KERN_ALERT "Bye world\n");
}

// Special kernel macros to indicate init and exit roles
module_init(hello_init);
module_exit(hello_exit);
