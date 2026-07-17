# Makefile - Exam PG3401 Task 4

# Points to the "include" folder
INCLDIR = ./include
CC      = gcc
CFLAGS  = -O2 -I$(INCLDIR) -include stddef.h

# Defines the obj folder, headers and object files
OBJDIR  = obj
_DEPS   = task4_prime.h
DEPS    = $(patsubst %,$(INCLDIR)/%,$(_DEPS))

_OBJS   = task4_threads.o task4_prime.o xtea.o ipc.o
OBJS    = $(patsubst %,$(OBJDIR)/%,$(_OBJS))

all: task4

$(OBJDIR)/task4_threads.o: task4_threads.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(OBJDIR)/task4_prime.o: task4_prime.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(OBJDIR)/xtea.o: xtea.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

# ipc.c uses C99 features (size_t in for loop) so it gets its own flag
$(OBJDIR)/ipc.o: ipc.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) -std=c99

# Links all object files into the final executable, -lpthread required for threads
task4: $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) -lpthread

.PHONY: clean
clean:
	rm -f $(OBJDIR)/*.o *~ core $(INCLDIR)/*~ task4
