obj-m := md1.o md2.o md3.o

KDIR = /lib/modules/$(shell uname -r)/build

default:
	make -C $(KDIR) M=$(shell pwd) modules
clean:
	make -C $(KDIR) M=$(shell pwd) clean


