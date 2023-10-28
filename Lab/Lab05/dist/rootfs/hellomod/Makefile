
obj-m     += hellomod.o
ccflags-y += -DEXPORT_SYMTAB

all: hello
	make -C ../../modulebuild M=$(PWD) modules

hello: hello.c
	$(CROSS_COMPILE)gcc -o $@ $< -Wall -static

clean:
	rm -f hello
	make -C ../../modulebuild M=$(PWD) clean

install: all
	mkdir -p ../modules
	cp hello hellomod.ko ../modules
