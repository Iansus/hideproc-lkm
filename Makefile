obj-m += hideproc.o
all:
	 make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	 make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

putfile: putfile.c
	gcc -o putfile putfile.c
