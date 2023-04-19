all:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD)/hello modules
	cp $(PWD)/hello/hello.ko . 2>/dev/null

clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD)/hello clean
	rm -f $(PWD)/hello.ko
