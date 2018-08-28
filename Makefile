obj-m += hideproc.o
all:
	 make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	 rm putfile
	 make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

putfile: putfile.c
	gcc -o putfile putfile.c

test:
	dmesg -C
	insmod hideproc.ko target_pid=$(PID) verb=1
	rmmod hideproc.ko
	dmesg -c
