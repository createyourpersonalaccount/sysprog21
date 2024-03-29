/* procfs1.c
 *
 * Demonstrates creating a file under /proc for communication, as well
 * as conditionals on kernel versions.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/version.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
/* proc_ops are used specifically for proc management, but are a newer
   feature. older kernels use the entire file_operations struct for
   this info. */
#define HAVE_PROC_OPS
#endif

#define PROCFS_NAME "helloworld"

static struct proc_dir_entry *our_proc_file;

static ssize_t procfile_read(struct file *file_pointer, char __user *buffer,
			     size_t buffer_length, loff_t *offset)
{
	char s[13] = "HelloWorld!\n";
	int len = sizeof(s);
	ssize_t ret = len;

	if (*offset >= len)
		ret = 0;
	else if (copy_to_user(buffer, s, len)) {
		pr_info("copy_to_user failed: %d\n", (int)*offset);
		ret = 0;
	} else {
		pr_info("procfile read %s\n",
			file_pointer->f_path.dentry->d_name.name);
		*offset += len;
	}

	return ret;
}

#ifdef HAVE_PROC_OPS
static const struct proc_ops proc_file_fops = {
	.proc_read = procfile_read,
};
#else
static const struct file_operations proc_file_fops = {
	.read = procfile_read,
};
#endif

static int __init procfs1_init(void)
{
	our_proc_file = proc_create(PROCFS_NAME, 0644, NULL, &proc_file_fops);
	if (NULL == our_proc_file) {
		proc_remove(our_proc_file);
		pr_alert("Error:Could not initialize /proc/%s\n", PROCFS_NAME);
		return -ENOMEM;
	}

	pr_info("/proc/%s created\n", PROCFS_NAME);
	return 0;
}

static void __exit procfs1_exit(void)
{
	proc_remove(our_proc_file);
	pr_info("/proc/%s removed\n", PROCFS_NAME);
}

module_init(procfs1_init);
module_exit(procfs1_exit);

MODULE_LICENSE("GPL");
