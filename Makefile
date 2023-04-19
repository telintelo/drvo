all: hello opdracht3
clean: clean-hello clean-opdracht3

hello:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD)/hello modules
	cp $(PWD)/hello/hello.ko . 2>/dev/null

clean-hello:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD)/hello clean
	rm -f $(PWD)/hello.ko

opdracht3:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD)/opdracht3 modules
	cp $(PWD)/opdracht3/opdracht3.ko . 2>/dev/null

clean-opdracht3:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD)/opdracht3 clean
	rm -f $(PWD)/opdracht3.ko

.PHONY: all clean hello clean-hello opdracht3 clean-opdracht3
