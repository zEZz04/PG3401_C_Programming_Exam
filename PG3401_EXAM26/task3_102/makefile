# Makefile - Exam PG3401 Task 3

# Points to the "include" folder
INCLDIR = ./include
CC      = gcc
CFLAGS  = -O2
CFLAGS += -I$(INCLDIR)
CFLAGS += -std=c89 -pedantic
CFLAGS += -Wall

# Defines the obj folder, headers and object files
OBJDIR = obj
_DEPS  = task3_header.h task3_util.h
DEPS   = $(patsubst %,$(INCLDIR)/%,$(_DEPS))

_OBJS  = task3_main.o task3_list.o task3_assessment.o task3_print.o task3_util.o
OBJS   = $(patsubst %,$(OBJDIR)/%,$(_OBJS))

$(OBJDIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

# Links all object files into the final executable
task3_program: $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean
clean:
	rm -f $(OBJDIR)/*.o *~ core $(INCLDIR)/*~ task3_program


