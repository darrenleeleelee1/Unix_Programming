CC = gcc
# CC = x86_64-linux-gnu-gcc
PROGS	= runner
all: $(PROGS)
runner: runner.c
	$(CC) runner.c -static-pie -g -o runner

cleanup:
	rm -f *.o

clean:
	rm -rf __pycache__/
	rm -f $(PROGS)
