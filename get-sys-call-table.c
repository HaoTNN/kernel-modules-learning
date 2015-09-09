#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/syscalls.h>

#define START_MEM PAGE_OFFSET
#define END_MEM ULLONG_MAX

// Special kernel macro to indicate license
MODULE_LICENSE("Dual BSD/GPL");

unsigned long ** sys_call_table;

static unsigned long ** get_sys_call_table(void)
{
	unsigned long i;
	unsigned long ** sctable;
	for (i = START_MEM; i < END_MEM; i += sizeof(void *))
	{
		sctable = (unsigned long **) i;
		if (sctable[__NR_close] == (unsigned long *) sys_close)
		{
			return sctable;
		}
	}	
	return NULL;
}

static int hijack_init(void)
{
	printk(KERN_INFO "Entering hijack module..\n");
	sys_call_table = get_sys_call_table();
	if (!sys_call_table)
	{
		printk(KERN_ERR "Unable to find system call table");
		return EPERM;
	}
	
	printk(KERN_INFO "Found sys_call_table at %x\n", sys_call_table);
	return 0;
}

static void hijack_exit(void)
{
	printk(KERN_INFO "Exiting hijack module..\n");
}

// Special kernel macros to indicate init and exit roles
module_init(hijack_init);
module_exit(hijack_exit);
