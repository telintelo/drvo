TARGET:=MAXLIN_LED

all: $(TARGET).dtbo

$(TARGET).dtbo: $(TARGET).idts
	dtc -O dtb -o $(TARGET).dtbo -Wno-unit_address_vs_reg $(TARGET).idts

$(TARGET).idts: $(TARGET).dts
	cpp -Iinclude -nostdinc -undef -D__DTS__ -x assembler-with-cpp -o $(TARGET).idts $(TARGET).dts

install: all
	cp -v $(TARGET).dtbo /lib/firmware

clean:
	rm -f $(TARGET).idts
	rm -f $(TARGET).dtbo

.PHONY: all clean install

