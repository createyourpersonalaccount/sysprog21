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

