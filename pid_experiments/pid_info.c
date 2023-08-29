#include <linux/mmap_lock.h>
#include <linux/proc_fs.h>
#include <linux/highmem.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/pid.h>
#include <linux/mm.h>

static int pid;
module_param(pid, int, 0);
MODULE_PARM_DESC(pid, "The process ID to print information of.");

static int __init my_init(void)
{
	pr_info("Hello world.\n");
	return 0;
}

static void __exit my_exit(void)
{
	pr_info("Goodbye world.\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");