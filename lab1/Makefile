obj-m = ch_drv.o
PWD = $(shell pwd)
all:
	make -C /lib/modules/$(shell uname -r)/build M="$(PWD)" modules
	rmmod ch_drv
	insmod ch_drv.ko
clean:
	make -C /lib/modules/$(shell uname -r)/build M="$(PWD)" clean
open:
	echo "open testfile" > /dev/var2
read:
	cat /dev/var2
close:
	echo "close" > /dev/var2

write:
	echo "test12" >> /dev/var2
