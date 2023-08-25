# Operations on modules

[Building external modules](https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/tree/Documentation/kbuild/modules.rst). Even more details about kernel makefiles [here](https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/tree/Documentation/kbuild/makefiles.rst).

- Inspect a `.ko` file with `modinfo`.
- Load a module with `insmod`.
  - Arguments can be passed via `insmod mymodule.ko variable=value` using [`module_param()`](https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/tree/include/linux/moduleparam.h). See hello-5.c. E.g.

        insmod hello-5.ko mystring="foo" myintarray=-1,3

- Remove a module with `rmmod`.
- View messages printed with `pr_info()` with `journalctl | tail`.
- `MODULE_LICENSE()` information [here](https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/tree/include/linux/module.h).
