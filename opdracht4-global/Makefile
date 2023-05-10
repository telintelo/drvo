TARGET_NAME := opdracht4

obj-m += ${TARGET_NAME}.o

all:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

insert: all
	-sudo rmmod ${TARGET_NAME}
	sudo insmod $(PWD)/${TARGET_NAME}.ko

remove:
	sudo rmmod ${TARGET_NAME}
