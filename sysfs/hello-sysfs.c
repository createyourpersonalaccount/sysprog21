/* hello-sysfs.c sysfs example
 *
 * Look into:
 *     $ /sys/kernel/mymodule/myvariable
 * Or write to:
 *    $ echo 42 > /sys/kernel/mymodule/myvariable
 *
 * kobject is the glue between the device model and the sysfs
 * interface.
 *
 * Note that the buffers are not tagged with __user; we deal with
 * kernel buffers here, and sysfs takes care for us of the user memory
 * aspect.
 */
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/sysfs.h>

static struct kobject *mymodule;

/* the variable you want to be able to change */
static int myvariable = 0;

static ssize_t myvariable_show(struct kobject *kobj,
			       struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", myvariable);
}

static ssize_t myvariable_store(struct kobject *kobj,
				struct kobj_attribute *attr, char *buf,
				size_t count)
{
	sscanf(buf, "%d", &myvariable);
	return count;
}

/* an attribute is a variable (in this case myvariable) with a mode
 * (0660 here).  Furthermore there's the operations show() and
 * store(): user reads from and writes to the attribute.
 *
 * This macro uses the variable `myvariable` to also name it
 * `"myvariable"`.
 *
 * Macro is equivalent to:
 *     { .attr = { .name = #_name,
 *                 .mode = _mode },
 *       .show = _show,
 *       .store = _store
 *     }
 */
static struct kobj_attribute myvariable_attribute =
	__ATTR(myvariable, 0660, myvariable_show, (void *)myvariable_store);

static int __init mymodule_init(void)
{
	int error = 0;

	pr_info("mymodule: initialised\n");

        /* Equivalent to:
         *     kobj = kobject_create();
         *     kobject_add(kobj, parent, "%s", name);
         * Must be freed with kobject_put().
         * Assigning @kernel_kobj as parent makes it lie under /sys/kernel.
         */
	mymodule = kobject_create_and_add("mymodule", kernel_kobj);
	if (!mymodule)
		return -ENOMEM;

        /* Equivalent to:
         *     sysfs_create_file_ns(kobj, attr, NULL);
         * Creates an attribute file for a kobject.
         * Its name is attr->name and its mode is attr->mode.
         *
         * This function is smart to figure out .show and .store from
         * the parent kobj_attribute.
         */
	error = sysfs_create_file(mymodule, &myvariable_attribute.attr);
	if (error) {
		pr_info("failed to create the myvariable file "
			"in /sys/kernel/mymodule\n");
	}

	return error;
}

static void __exit mymodule_exit(void)
{
	pr_info("mymodule: Exit success\n");
        /* Decrease the reference counter of the kobject; since we're
           the sole owner, this frees the object. */
	kobject_put(mymodule);
}

module_init(mymodule_init);
module_exit(mymodule_exit);

MODULE_LICENSE("GPL");
