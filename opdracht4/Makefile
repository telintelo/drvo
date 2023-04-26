obj-m += opdracht4.o

all:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	cp $(PWD)/opdracht4.ko .. 2>/dev/null

clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f $(PWD)/../opdracht4.ko

insert: all
	-sudo rmmod opdracht4
	sudo insmod $(PWD)/opdracht4.ko

remove:
	sudo rmmod opdracht4
