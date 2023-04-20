obj-m += hello.o

all:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	cp $(PWD)/hello.ko .. 2>/dev/null

clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f $(PWD)/../hello.ko
