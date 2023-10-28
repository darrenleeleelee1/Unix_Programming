
PROGS	= solver
all: $(PROGS)
solver: solver.c
	x86_64-linux-gnu-gcc solver.c -g -o solver

cleanup:
	rm -f *.o

clean:
	rm -rf __pycache__/
	rm -f $(PROGS)
