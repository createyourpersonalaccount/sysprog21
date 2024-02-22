/* \file chardev2.h
 *
 * The ioctl definitions.
 *
 * The definitions here have to be in a header file, because they need
 * to be known both to the kernel module (in chardev2.c) and the process
 * calling ioctl() (in userspace/main.c).
 */

#ifndef CHARDEV2_H_
#define CHARDEV2_H_

#include <linux/ioctl.h>

/* The major device number. We can not rely on dynamic registration
 * any more, because ioctls need to know it.
 */
#define MAJOR_NUM 100
/* Set the message of the device driver
 *
 * _IOW means that we are creating an ioctl command number for passing
 * information from a user process to the kernel module.
 *
 * The first arguments, MAJOR_NUM, is the major device number we are using.
 *
 * The second argument is the number of the command (there could be several
 * with different meanings).
 *
 * The third argument is the type we want to get from the process to the
 * kernel.
 */
#define IOCTL_SET_MSG _IOW(MAJOR_NUM, 0, char *)
/* Get the message of the device driver
 * This IOCTL is used for output, to get the message of the device driver.
 * However, we still need the buffer to place the message in to be input,
 * as it is allocated by the process.
 */
#define IOCTL_GET_MSG _IOR(MAJOR_NUM, 1, char *)
/* Get the n'th byte of the message
 * The IOCTL is used for both input and output. It receives from the user
 * a number, n, and returns message[n].
 */
#define IOCTL_GET_NTH_BYTE _IOWR(MAJOR_NUM, 2, int)
/* The name of the device file */
#define DEVICE_NAME "chardev2"

#endif /* CHARDEV2_H_ */
