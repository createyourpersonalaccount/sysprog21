/* \file chardev2.c
 *
 * Create an input/output character device.
 */

#include <chardev2_private.h>
#include <linux/cdev.h>
#include <linux/fs.h>

static atomic_t already_open = ATOMIC_INIT(CDEV_NOT_USED);
static char message[BUF_LEN + 1];
static struct class *cls;

static int device_open(struct inode *inode, struct file *file)
{
	pr_info("%s: device_open(%p,%p)\n", DEVICE_NAME, inode, file);
	/* Increment the reference count of a module. See
           <https://lwn.net/Articles/22197/> */
	if (!try_module_get(THIS_MODULE)) {
		return -EINVAL;
	} else {
		return 0;
	}
}

static int device_release(struct inode *inode, struct file *file)
{
	pr_info("%s: device_release(%p,%p)\n", DEVICE_NAME, inode, file);
	/* Decrement the reference count of a module. */
	module_put(THIS_MODULE);
	return 0;
}

static ssize_t device_read(struct file *file, /* see include/linux/fs.h   */
			   char __user *buffer, /* buffer to be filled  */
			   size_t length, /* length of the buffer     */
			   loff_t *offset)
{
	/* Number of bytes actually written to the buffer */
	int bytes_read = 0;
	/* How far did the process reading the message get? Useful if the message
         * is larger than the size of the buffer we get to fill in device_read.
         */
	const char *message_ptr = message;
	if (!*(message_ptr + *offset)) { /* we are at the end of message */
		*offset = 0; /* reset the offset */
		return 0; /* signify end of file */
	}
	message_ptr += *offset;
	/* Actually put the data into the buffer */
	while (length && *message_ptr) {
		/* Because the buffer is in the user data segment, not the kernel
                 * data segment, assignment would not work. Instead, we have to
                 * use put_user which copies data from the kernel data segment to
                 * the user data segment.
                 */
		put_user(*(message_ptr++), buffer++);
		length--;
		bytes_read++;
	}
	pr_info("%s: Read %d bytes, %ld left\n", DEVICE_NAME, bytes_read,
		length);
	*offset += bytes_read;
	/* Read functions are supposed to return the number of bytes actually
         * inserted into the buffer.
         */
	return bytes_read;
}

/* called when somebody tries to write into our device file. */
static ssize_t device_write(struct file *file, const char __user *buffer,
			    size_t length, loff_t *offset)
{
	int i;
	pr_info("%s: device_write(%p,%p,%ld)", DEVICE_NAME, file, buffer,
		length);
	for (i = 0; i < length && i < BUF_LEN; i++)
		get_user(message[i], buffer + i);
	/* Again, return the number of input characters used. */
	return i;
}

/* This function is called whenever a process tries to do an ioctl on our
 * device file. We get two extra parameters (additional to the inode and file
 * structures, which all device functions get): the number of the ioctl called
 * and the parameter given to the ioctl function.
 *
 * If the ioctl is write or read/write (meaning output is returned to the
 * calling process), the ioctl call returns the output of this function.
 */
static long
device_ioctl(struct file *file, /* ditto */
	     unsigned int ioctl_num, /* number and param for ioctl */
	     unsigned long ioctl_param)
{
	int i;
	long ret = 0;
	/* We don't want to talk to two processes at the same time. */
	if (atomic_cmpxchg(&already_open, CDEV_NOT_USED, CDEV_EXCLUSIVE_OPEN))
		return -EBUSY;
	/* Switch according to the ioctl called */
	switch (ioctl_num) {
	case IOCTL_SET_MSG: {
		/* Receive a pointer to a message (in user space) and set that to
                 * be the device's message. Get the parameter given to ioctl by
                 * the process.
                 */
		char __user *tmp = (char __user *)ioctl_param;
		char ch;
		/* Find the length of the message */
		get_user(ch, tmp);
		for (i = 0; ch && i < BUF_LEN; i++, tmp++)
			get_user(ch, tmp);
		device_write(file, (char __user *)ioctl_param, i, NULL);
		break;
	}
	case IOCTL_GET_MSG: {
		loff_t offset = 0;
		/* Give the current message to the calling process - the parameter
                 * we got is a pointer, fill it.
                 */
		i = device_read(file, (char __user *)ioctl_param, 99, &offset);
		/* Put a zero at the end of the buffer, so it will be properly
                 * terminated.
                 */
		put_user('\0', (char __user *)ioctl_param + i);
		break;
	}
	case IOCTL_GET_NTH_BYTE:
		/* This ioctl is both input (ioctl_param) and output (the return
                 * value of this function).
                 */
		ret = (long)message[ioctl_param];
		break;
	}
	/* We're now ready for our next caller */
	atomic_set(&already_open, CDEV_NOT_USED);
	return ret;
}

static struct file_operations fops = {
	.read = device_read,
	.write = device_write,
	.unlocked_ioctl = device_ioctl,
	.open = device_open,
	.release = device_release, /* a.k.a. close */
};

static int __init chardev2_init(void)
{
	/* Try to register the character device */
	int ret_val = register_chrdev(MAJOR_NUM, DEVICE_NAME, &fops);
	if (ret_val < 0) {
		pr_alert(
			"%s: Registering the character device failed with %d\n",
			DEVICE_NAME, ret_val);
		return ret_val;
	}
	cls = class_create(THIS_MODULE, DEVICE_NAME);
	device_create(cls, NULL, MKDEV(MAJOR_NUM, 0), NULL, DEVICE_NAME);
	pr_info("%s: Device created on /dev/%s.\n", DEVICE_NAME, DEVICE_NAME);
	return 0;
}

static void __exit chardev2_exit(void)
{
	device_destroy(cls, MKDEV(MAJOR_NUM, 0));
	class_destroy(cls);
	unregister_chrdev(MAJOR_NUM, DEVICE_NAME);
	pr_info("%s: Exiting.\n", DEVICE_NAME);
}

module_init(chardev2_init);
module_exit(chardev2_exit);

MODULE_LICENSE("GPL");
