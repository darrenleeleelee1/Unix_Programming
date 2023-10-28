
obj-m     += kshram.o
ccflags-y += -DEXPORT_SYMTAB

all: kshram_test
	make -C ../../modulebuild M=$(PWD) modules

kshram_test: kshram_test.c
	$(CROSS_COMPILE)gcc -o $@ $< -Wall -static

clean:
	rm -f kshram_test
	make -C ../../modulebuild M=$(PWD) clean

install: all
	mkdir -p ../modules
	cp kshram_test kshram.ko ../modules