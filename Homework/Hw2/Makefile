CC = gcc
# CC = x86_64-linux-gnu-gcc
PROGS	= sdb
all: $(PROGS)
sdb: sdb.c
	$(CC) sdb.c -I/usr/include/capstone -static-pie -lcapstone -g -o sdb

cleanup:
	rm -f *.o

clean:
	rm -rf __pycache__/
	rm -f $(PROGS)
