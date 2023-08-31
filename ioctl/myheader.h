#ifndef MYHEADER_IOCTL_H_
#define MYHEADER_IOCTL_H_

static unsigned int test_ioctl_major = 0;
static unsigned int num_of_dev = 1;
static struct cdev test_ioctl_cdev;
static int ioctl_num = 0;

struct ioctl_arg {
	unsigned int val;
};

/* Documentation/userspace-api/ioctl/ioctl-number.txt */

/* Our IOCTL magic number. */
#define IOC_MAGIC '\x66'

/* ioctl's have a magic number, a sequence number, and a data type being passed around. */

/* ioctl with write parameters from user. */
#define IOCTL_VALSET _IOW(IOC_MAGIC, 0, struct ioctl_arg)
/* ioctl with read parameters from user. */
#define IOCTL_VALGET _IOR(IOC_MAGIC, 1, struct ioctl_arg)

/* Ditto. */
#define IOCTL_VALGET_NUM _IOR(IOC_MAGIC, 2, int)
#define IOCTL_VALSET_NUM _IOW(IOC_MAGIC, 3, int)

#define IOCTL_VAL_MAXNR 3
#define DRIVER_NAME "ioctltest"

struct my_data {
	unsigned char val;
	rwlock_t lock;
};

static long my_unlocked_ioctl(struct file *filp, unsigned int cmd,
			     unsigned long arg);
static ssize_t my_read(struct file *filp, char __user *buf,
			       size_t count, loff_t *f_pos);
static int my_close(struct inode *inode, struct file *filp);
static int my_open(struct inode *inode, struct file *filp);

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = my_open,
	.release = my_close,
	.read = my_read,
	.unlocked_ioctl = my_unlocked_ioctl,
};

#endif /* MYHEADER_IOCTL_H_ */
