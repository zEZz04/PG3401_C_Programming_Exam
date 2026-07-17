# Makefile for Task 5 - EWP Server
INCLDIR = ./include
CC      = gcc

# Compiler and flags
CFLAGS  = -O2 -Wall -Wextra -ansi -pedantic
CFLAGS += -I$(INCLDIR)

OBJDIR = obj

# Headers and object files
_DEPS = ewpdef.h clienthandler.h server.h util.h
DEPS  = $(patsubst %,$(INCLDIR)/%,$(_DEPS))

_OBJS = main.o server.o clienthandler.o util.o
OBJS  = $(patsubst %,$(OBJDIR)/%,$(_OBJS))

# Compile each .c file into a .o file in the obj directory
$(OBJDIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

# Link all object files into the final binary
task5: $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean
clean:
	rm -f $(OBJDIR)/*.o *~ core $(INCLDIR)/*~ task5
