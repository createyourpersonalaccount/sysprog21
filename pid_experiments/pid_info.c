#include <linux/mmap_lock.h>
#include <linux/proc_fs.h>
#include <linux/highmem.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/pid.h>
#include <linux/mm.h>

#define MODULE_NAME "pid_info"

static int pid;
module_param(pid, int, 0);
MODULE_PARM_DESC(pid, "The process ID for which we print information.");

static int __init my_init(void)
{
	unsigned int level;
	struct pid *vpid;
	rcu_read_lock();
	vpid = find_vpid(pid);
	/* do something with vpid */
	vpid = rcu_dereference(vpid);
	if (vpid == NULL) {
		rcu_read_unlock();
		pr_info("%s: find_vpid(%d) failed.\n", MODULE_NAME, pid);
	} else {
		level = vpid->level;
		rcu_read_unlock();
		pr_info("%s: vpid->level: %u\n", MODULE_NAME, level);
	}
	return 0;
}

static void __exit my_exit(void)
{
	pr_info("%s: Goodbye world.\n", MODULE_NAME);
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
