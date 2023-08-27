# Operations on modules

[Building external modules](https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/tree/Documentation/kbuild/modules.rst). Even more details about kernel makefiles [here](https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/tree/Documentation/kbuild/makefiles.rst).

- Inspect a `.ko` file with `modinfo`.
- Load a module with `insmod`.
  - Arguments can be passed via `insmod mymodule.ko variable=value` using [`module_param()`](https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/tree/include/linux/moduleparam.h). See hello-5.c. E.g.

        insmod hello-5.ko mystring="foo" myintarray=-1,3

- Remove a module with `rmmod`.
- View how many processes use a module with `lsmod`.
- View messages printed with `pr_info()` with `journalctl | tail`.
- `MODULE_LICENSE()` information [here](https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/tree/include/linux/module.h).

Kernel modules are object files whose symbols are resolved by `insmod` (or `modprobe` for already-installed modules.) Exported kernel symbols are in `/proc/kallsyms`.

# Device drivers

Device drivers are a class of kernel modules providing functionality for hardware. 

Device files are under `/dev`; they provide means of communicating with hardware. This provides a general method of communicating with drivers: `/dev/sound` may be connected to by the `es1370.ko` driver, or some other. Device files can be created by e.g. `mknod /dev/coffee c 12 2`.

Major and minor numbers ([Assigned major/minor number listing](https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/tree/Documentation/admin-guide/devices.txt)) are listed by `ls -l` in the form `MAJOR, MINOR`. The major number is the corresponding device driver controlling it, and the minor number is to differentiate different (potentially abstract) hardware. When a device file is accessed, the kernel determines the module controlling it by the major number. The minor number is for the module itself to consume. Major numbers and drivers currently online are under `/proc/devices`.

Device files are either "character" or "block". Block devices have IO in blocks of bytes and can buffer requests, while character devices work with bytes.

## Character devices

The [`struct file_operations`](https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/tree/include/linux/fs.h) holds operation callbacks such as `read()` and `write()`. Unused operations are set to `NULL`. The variable is commonly called `fops`. `struc proc_ops` replaces `struct file_operations` for merely registering `/proc` handlers.

The `struct file` represents an abstract open file.

To register a major number for a character device, use either `register_chrdev_region()` or `alloc_chrdev_region()`. The former is with a fixed major number and the latter dynamically allocates one that is available. Then the functions `cdev_alloc()`, `cdev_init()`, `cdev_add()`, etc, are used.


In `<linux/module.h>`, the following functions are available to view or modify the use counter:

    try_module_get()  /* Increment the reference count of current module. */
    module_put()      /* Decrement the reference count of current module. */
    module_refcount() /* Return the value of reference count of current module. */


# Conditional compilation for different kernel versions

    /* Conditionally compile for kernel 2.6.16 or less */
    #if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,16)
      /* ... */
    #endif

# Examples

## `chardev`

We define four functions, `device_{open,release,read,write}`, which we populate a `struct file_operations` with.

In our init function, we register a character device with `register_chrdev` so that the kernel allocates a major number for us. We have

    cls = class_create(THIS_MODULE, "chardev");
    device_create(cls, NULL, MKDEV(major, 0), NULL, "chardev");

I don't know what `class_create` does, but `cls` must be deallocated with `class_destroy()`; `THIS_MODULE` is a macro to a struct and `"chardev"` is the name of the device file, which `device_create` creates. `MKDEV()` combines a major and a minor number.

The four registered functions are called when a process opens/closes/reads or writes the file.

Because we want to synchronize different processes, we use `<linux/atomic.h>` and `ATOMIC_INIT(val)`, `atomic_cmpxchg(&x, comp, newval)`, and `atomic_set(&x, val)`.

We use `try_module_get(THIS_MODULE)` and `module_put(THIS_MODULE)` to let the kernel know yet another process is using the module, preventing the module from exiting prematurely.

Now `try_module_get()` presents an issue, and there is a superior alternative. See [SA/a/6079839](https://stackoverflow.com/a/6079839).

Writing to the device fails with `-EINVAL`.

Reading from the device essentially calls `put_user(*msg++, *buf++)` over and over until the whole message is written, and returns the number of bytes. The function `put_user()` copies from kernel memory to user memory, note it is tagged with `char __user *buf`.
