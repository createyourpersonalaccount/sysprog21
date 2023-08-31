/* ioctl.c
 */
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/ioctl.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#include "myheader.h"

static long my_unlocked_ioctl(struct file *filp, unsigned int cmd,
			      unsigned long arg)
{
	/* This is where the char devices output byte lies. */
	struct my_data *ioctl_data = filp->private_data;
	int retval = 0;
	unsigned char val;
	struct ioctl_arg data;
	memset(&data, 0, sizeof(data));

        pr_alert("%s call.\n", __func__);

	switch (cmd) {
	case IOCTL_VALSET:
		/* @arg is the pointer to the ioctl data type (struct
                   ioctl_arg). NB perhaps the cast should be (struct
                   ioctl_arg __user *) instead of (int __user *).  */
		if (copy_from_user(&data, (int __user *)arg, sizeof(data))) {
			retval = -EFAULT;
			goto done;
		}

		pr_alert("IOCTL set val:%x .\n", data.val);
		write_lock(&ioctl_data->lock);
		ioctl_data->val = data.val;
		write_unlock(&ioctl_data->lock);
		break;

	case IOCTL_VALGET:
		read_lock(&ioctl_data->lock);
		val = ioctl_data->val;
		read_unlock(&ioctl_data->lock);
		data.val = val;

		if (copy_to_user((int __user *)arg, &data, sizeof(data))) {
			retval = -EFAULT;
			goto done;
		}

		break;

	case IOCTL_VALGET_NUM:
		retval = __put_user(ioctl_num, (int __user *)arg);
		break;

	case IOCTL_VALSET_NUM:
		ioctl_num = arg;
		break;

	default:
		retval = -ENOTTY;
	}

done:
	return retval;
}

/* If a user reads from this device file, it never ends, always
   outputting the same byte value. */
static ssize_t my_read(struct file *filp, char __user *buf, size_t count,
		       loff_t *f_pos)
{
	struct my_data *ioctl_data = filp->private_data;
	unsigned char val;
	int retval;
	int i = 0;

	/* Lock to retrieve value ... */
	read_lock(&ioctl_data->lock);
	val = ioctl_data->val;
	read_unlock(&ioctl_data->lock);

	/* ... and fill user buffer with it. */
	for (; i < count; i++) {
		if (copy_to_user(&buf[i], &val, 1)) {
			retval = -EFAULT;
			goto out;
		}
	}

	retval = count;
out:
	return retval;
}

static int my_close(struct inode *inode, struct file *filp)
{
	pr_alert("%s call.\n", __func__);

	if (filp->private_data) {
		kfree(filp->private_data);
		filp->private_data = NULL;
	}

	return 0;
}

static int my_open(struct inode *inode, struct file *filp)
{
	struct my_data *ioctl_data;

	pr_alert("%s call.\n", __func__);
	/* GFP_KERNEL is for kernel-internal memory. See
           <linux/gfp_types.h>. */
	ioctl_data = kmalloc(sizeof(struct my_data), GFP_KERNEL);

	if (ioctl_data == NULL)
		return -ENOMEM;

	/* Initialize the lock, make up a value, and save to filp's
           private_data field. */
	rwlock_init(&ioctl_data->lock);
	ioctl_data->val = 0xFF;
	filp->private_data = ioctl_data;

	return 0;
}

static int __init ioctl_init(void)
{
	/* an integer encoding MAJOR and MINOR */
	dev_t dev;

	int alloc_ret = -1;
	int cdev_ret = -1;
	/* Allocate some numbers for char dev registration */
	alloc_ret = alloc_chrdev_region(&dev, 0, num_of_dev, DRIVER_NAME);

	if (alloc_ret)
		goto error;

	/* In the lines below, we create a character device. */
	test_ioctl_major = MAJOR(dev);
	/* Initialize the cdev struct with fops. */
	cdev_init(&test_ioctl_cdev, &fops);
	/* Add char device to system. */
	cdev_ret = cdev_add(&test_ioctl_cdev, dev, num_of_dev);

	if (cdev_ret)
		goto error;

	pr_alert("%s driver(major: %d) installed.\n", DRIVER_NAME,
		 test_ioctl_major);
	return 0;
error:
	if (cdev_ret == 0)
		cdev_del(&test_ioctl_cdev);
	if (alloc_ret == 0)
		unregister_chrdev_region(dev, num_of_dev);
	return -1;
}

static void __exit ioctl_exit(void)
{
	/* We know that the minor number is 0 because we fixed that
         * magic minor number in the alloc_chrdev_region()
         * invocation. */
	dev_t dev = MKDEV(test_ioctl_major, 0);

	cdev_del(&test_ioctl_cdev);
	unregister_chrdev_region(dev, num_of_dev);
	pr_alert("%s driver removed.\n", DRIVER_NAME);
}

module_init(ioctl_init);
module_exit(ioctl_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("This is test_ioctl module");
