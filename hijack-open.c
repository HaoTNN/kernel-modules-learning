#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <asm/current.h>

#define START_MEM PAGE_OFFSET // Where kernel memory is defined to start at
#define END_MEM ULLONG_MAX // Ending point

// In order to replace system calls in the system call table,
// need to disable the write protection on it.
#define DISABLE_WRITE_PROT (write_cr0 (read_cr0() & (~0x10000)));
#define ENABLE_WRITE_PROT (write_cr0 (read_cr0() | 0x10000));

unsigned long ** sys_call_table;

asmlinkage int (*original_open) (const char* pathname, int flags, mode_t mode);

static unsigned long ** get_sys_call_table(void)
{
	unsigned long i;
	unsigned long **sctable;
	for (i = START_MEM; i < END_MEM; i += sizeof(void *))
	{
		sctable = (unsigned long **) i;
		if (sctable[__NR_close] == (unsigned long *) sys_close) // Other system calls were found to be "undefined"
		{
			return sctable;
		}
	}
	return NULL;
}

asmlinkage int custom_open(const char* pathname, int flags, mode_t mode)
{
	printk(KERN_INFO "sys_open called by \"%s\", PID: %i\n", current->comm, current->pid);
	return (*original_open)(pathname, flags, mode);
}

static int hijack_init(void)
{
	printk(KERN_INFO "Hijack init..");
	sys_call_table = get_sys_call_table();
	if (!sys_call_table)
	{
		printk(KERN_ERR "Unable to locate system call table");
		return EPERM;
	}

	DISABLE_WRITE_PROT;
	original_open = (void *)sys_call_table[__NR_open];
	sys_call_table[__NR_open] = (unsigned long *) custom_open;
	ENABLE_WRITE_PROT;

	return 0;
}

static void hijack_exit(void)
{
	printk(KERN_INFO "Hijack exiting..");

	DISABLE_WRITE_PROT;
	sys_call_table[__NR_open] = (unsigned long *) original_open;
	ENABLE_WRITE_PROT;
}

module_init(hijack_init);
module_exit(hijack_exit);
