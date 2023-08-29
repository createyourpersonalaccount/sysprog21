#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/sysfs.h>

#define SYSFS_DIR_NAME "mymodule"

static struct kobject *mymodule;

static int __init mymodule_init(void)
{
        /* Allocate a kobject and set its name and parent.  Since the
         parent is kernel_kobj, it is added under the /sys/kernel
         directory.  */
	mymodule = kobject_create_and_add(SYSFS_DIR_NAME, kernel_kobj);
	if (!mymodule)
		return -ENOMEM;
        pr_info("created: /sys/kernel/%s\n", SYSFS_DIR_NAME);
	return 0;
}

static void __exit mymodule_exit(void)
{
        /* Decrease the reference counter of the kobject; since we're
           the sole owner, this frees the object. */
	kobject_put(mymodule);
        pr_info("deleted: /sys/kernel/%s\n", SYSFS_DIR_NAME);
}

module_init(mymodule_init);
module_exit(mymodule_exit);

MODULE_LICENSE("GPL");

