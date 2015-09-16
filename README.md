Learning Kernel Modules
=======================

## Summary
These are kernel modules I've written in an attempt to learn more about the Linux Kernel and how modules work.
Obviously, these shouldn't be used for anything else other than learning.

## Modules
**'hello-world.c'**: Basic "hello world" module.

**'get-sys-call-table.c'**: Gets the address of the system call table and logs it.

**'hijack-open.c'**: Replaces the original open() system call with a custom open(). The custom call will open a pre-defined file in place of the file that is
supposed to open. The call will only open the pre-defined file if the original file has an extension that matches a pre-defined extension in the module.

**'hijack-.c'**: Replaces the original close() system call with a custom close(). The custom call will write a pre-defined string to a file before closing it.
The call will only write to the file if the extension matches a pre-defined extension in the module.
