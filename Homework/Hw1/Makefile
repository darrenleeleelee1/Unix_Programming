
CC		= gcc
CFLAGS	= -Wall -g -O0 -D_GNU_SOURCE -fPIC
PROGS	= sandbox.so
EXAMPLE_1 = ./launcher ./sandbox.so config.txt cat /etc/passwd
EXAMPLE_2 = ./launcher ./sandbox.so config.txt cat /etc/hosts
EXAMPLE_3 = ./launcher ./sandbox.so config.txt cat /etc/ssl/certs/Amazon_Root_CA_1.pem
EXAMPLE_5 = ./launcher ./sandbox.so config.txt wget http://google.com -t 1
EXAMPLE_6 = ./launcher ./sandbox.so config.txt wget https://www.nycu.edu.tw -t 1
EXAMPLE_7 = ./launcher ./sandbox.so config.txt wget http://www.google.com -q -t 1
EXAMPLE_8 = ./launcher ./sandbox.so config.txt python3 -c 'import os;os.system("wget http://www.google.com -q -t 1")'
COMMAND = $(EXAMPLE_8)
all: $(PROGS)

%.o: %.c
	$(CC) -c $< $(CFLAGS)

sandbox.so: sandbox.o got.o function_list.o config.o
	$(CC) -o $@ -shared $^ 

run:
	$(COMMAND)
test:
	$(COMMAND)
	cat ./debug.log
debug:
	gdb --args $(COMMAND)

clean:
	rm -f $(PROGS) *.o *.log **.html**
	rm -rf __pycache__

