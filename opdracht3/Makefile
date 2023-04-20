obj-m += opdracht3.o

all:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	cp $(PWD)/opdracht3.ko .. 2>/dev/null

clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f $(PWD)/../opdracht3.ko
