obj-m = ch_drv.o
PWD = $(shell pwd)
all:
	make -C /lib/modules/$(shell uname -r)/build M="$(PWD)" modules
	rmmod ch_drv
	insmod ch_drv.ko
clean:
	make -C /lib/modules/$(shell uname -r)/build M="$(PWD)" clean
open:
	echo "open testfile" > /dev/var1
read:
	cat /dev/var1
close:
	echo "close" > /dev/var1

write:
	echo "test1" > /dev/var1