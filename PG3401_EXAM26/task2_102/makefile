# Makefile - Exam PG3401 Task 2

# Path to the directory containing header files
INCLDIR = ./include

CC = gcc

CFLAGS = -O2
CFLAGS += -I$(INCLDIR)
CFLAGS += -std=c89 -Wall

# Directory where compiled object files will be placed
OBJDIR = obj

_DEPS = task2_header.h
DEPS = $(patsubst %,$(INCLDIR)/%,$(_DEPS))

_OBJS = task2_main.o task2_fib.o task2_prim.o task2_kvad.o task2_cube.o task2_perf.o task2_abun.o task2_def.o task2_odd.o
OBJS = $(patsubst %,$(OBJDIR)/%,$(_OBJS))

$(OBJDIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

# Link all object files into the final executable
task2_program: $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) -lm

.PHONY: clean

clean:
	rm -f $(OBJDIR)/*.o *~ core $(INCLDIR)/*~ task2_program pgexam26_output.bin
