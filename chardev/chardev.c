/* \file chardev.c
 *
 * Creates a read-only char device counter.
 *
 * Reading from the device will give you a counter of how many times
 * the device has been read.
 *
 * \code{.sh}
 * cat /dev/chardev
 * \endcode
 *
 * Print messages on module loading and exit can be seen with \c
 * dmesg(1). Attempts to write to the device can also be viewed with
 * \c dmesg(1):
 *
 * \code{.sh}
 * echo bad > /dev/chardev
 * \endcode
 */

#include <linux/cdev.h>
#include <linux/fs.h>

/* Function prototypes - these would normally go in a header. */
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char __user *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char __user *, size_t,
			    loff_t *);

/* Dev name as it appears in /proc/devices   */
#define DEVICE_NAME "chardev"
/* Max length of the message from the device */
#define BUF_LEN 80
/* major number assigned to our device driver */
static int major;
/* Possible values of the binary semaphore. */
enum {
	CDEV_NOT_USED = 0,
	CDEV_EXCLUSIVE_OPEN = 1,
};
/* Is device open? An atomic binary semaphore used to prevent concurrent
   access to device. */
static atomic_t already_open = ATOMIC_INIT(CDEV_NOT_USED);
/* The msg the device will give when asked. */
static char msg[BUF_LEN + 1];
/* See <https://lwn.net/Articles/128644/>. */
static struct class *cls;
/* This structure holds the functions to be called when a process does
 * something to the device we created. Since a pointer to this structure
 * is kept in the devices table, it can't be local to init_module. NULL is
 * for unimplemented functions.
 */
static struct file_operations chardev_fops = {
	.owner = THIS_MODULE,
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release,
};

static int __init chardev_init(void)
{
	major = register_chrdev(0, DEVICE_NAME, &chardev_fops);
	if (major < 0) {
		pr_alert("%s: Registering char device failed with %d\n",
			 DEVICE_NAME, major);
		return major;
	}
	pr_info("%s: I was assigned major number %d.\n", DEVICE_NAME, major);
	cls = class_create(THIS_MODULE, DEVICE_NAME);
	device_create(cls, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
	pr_info("%s: Device file created on /dev/%s\n", DEVICE_NAME,
		DEVICE_NAME);
	return 0;
}

static void __exit chardev_exit(void)
{
	device_destroy(cls, MKDEV(major, 0));
	class_destroy(cls);
	/* Unregister the device */
	unregister_chrdev(major, DEVICE_NAME);
	pr_info("%s: Exiting.\n", DEVICE_NAME);
}

/* Methods */

/* Called when a process tries to open the device file, like
 * "sudo cat /dev/chardev"
 */
static int device_open(struct inode *inode, struct file *file)
{
	static int counter = 0;
	if (atomic_cmpxchg(&already_open, CDEV_NOT_USED, CDEV_EXCLUSIVE_OPEN))
		return -EBUSY;
	sprintf(msg, "I already told you %d times Hello world!\n", counter++);
	return 0;
}

/* Called when a process closes the device file. */
static int device_release(struct inode *inode, struct file *file)
{
	/* We're now ready for our next caller */
	atomic_set(&already_open, CDEV_NOT_USED);
	return 0;
}

/* Called when a process, which already opened the dev file, attempts to
 * read from it.
 */
static ssize_t device_read(struct file *filp, /* see include/linux/fs.h */
			   char __user *buffer, /* buffer to fill with data */
			   size_t length, /* length of the buffer */
			   loff_t *offset)
{
	/* Number of bytes actually written to the buffer */
	int bytes_read = 0;
	const char *msg_ptr = msg;
	if (msg_ptr[*offset] == '\0') { /* we are at the end of message */
		*offset = 0; /* reset the offset */
		return 0; /* signify end of file */
	}
	msg_ptr += *offset;
	/* Actually put the data into the buffer */
	while (length && *msg_ptr != '\0') {
		/* The buffer is in the user data segment, not the
                 * kernel segment so
                 *
                 *     *buffer++ = *msg_ptr++;
                 *
                 * won't work.  We have to use put_user which copies
                 * data from the kernel data segment to the user data
                 * segment.
                 */
		put_user(*(msg_ptr++), buffer++);
		length--;
		bytes_read++;
	}
	*offset += bytes_read;
	/* Most read functions return the number of bytes put into the buffer. */
	return bytes_read;
}

/* Called when a process writes to dev file: echo "hi" > /dev/hello */
static ssize_t device_write(struct file *filp, const char __user *buff,
			    size_t len, loff_t *off)
{
	pr_alert("%s: Write operations are not supported.\n", DEVICE_NAME);
	return -EINVAL;
}

module_init(chardev_init);
module_exit(chardev_exit);

MODULE_LICENSE("GPL");
