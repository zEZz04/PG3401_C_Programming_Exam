# Makefile for EWA TCP-over-TCP client (task6)
INCLDIR = ./include
CC      = gcc
CFLAGS  = -O2 -Wall -Wextra -pedantic -std=c89
CFLAGS += -I$(INCLDIR)

OBJDIR = obj

# Headers and object files
_DEPS = client.h ewpdef.h
DEPS  = $(patsubst %,$(INCLDIR)/%,$(_DEPS))

_OBJS = main.o client.o
OBJS  = $(patsubst %,$(OBJDIR)/%,$(_OBJS))

# Compile each .c file into a .o file in the obj directory
$(OBJDIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

# Link all object files into the final binary
task6: $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean
clean:
	rm -f $(OBJDIR)/*.o *~ core $(INCLDIR)/*~ task6 task6_received.bmp
