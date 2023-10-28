CXXPROC = gcc
# CPPFLAGS = -O3 -Wall
CPPFLAGS = -O3 -Wall -static
PROG_TARGET = solver


all: $(PROG_TARGET)
$(PROG_TARGET): solver.c
	$(CXXPROC) -o $@ $(CPPFLAGS) $<
