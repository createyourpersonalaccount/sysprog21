#ifndef MYHEADER_IOCTL_H_
#define MYHEADER_IOCTL_H_

static unsigned int test_ioctl_major = 0;
static unsigned int num_of_dev = 1;
static struct cdev test_ioctl_cdev;
static int ioctl_num = 0;

struct ioctl_arg {
	unsigned int val;
};

/* Documentation/ioctl/ioctl-number.txt */
#define IOC_MAGIC '\x66'

#define IOCTL_VALSET _IOW(IOC_MAGIC, 0, struct ioctl_arg)
#define IOCTL_VALGET _IOR(IOC_MAGIC, 1, struct ioctl_arg)
#define IOCTL_VALGET_NUM _IOR(IOC_MAGIC, 2, int)
#define IOCTL_VALSET_NUM _IOW(IOC_MAGIC, 3, int)

#define IOCTL_VAL_MAXNR 3
#define DRIVER_NAME "ioctltest"

struct test_ioctl_data {
	unsigned char val;
	rwlock_t lock;
};

static long test_ioctl_ioctl(struct file *filp, unsigned int cmd,
			     unsigned long arg);
static ssize_t test_ioctl_read(struct file *filp, char __user *buf,
			       size_t count, loff_t *f_pos);
static int test_ioctl_close(struct inode *inode, struct file *filp);
static int test_ioctl_open(struct inode *inode, struct file *filp);

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = test_ioctl_open,
	.release = test_ioctl_close,
	.read = test_ioctl_read,
	.unlocked_ioctl = test_ioctl_ioctl,
};

#endif /* MYHEADER_IOCTL_H_ */
