CC = gcc # compiler
CFLAGS = -Wall -g # compile flags
LIBS = -lpthread -lrt # libs

TARGETS = seq par_t par_p

all: $(TARGETS)

seq: seq.o
	$(CC) -o seq seq.o $(LIBS)

par_t: par_t.o
	$(CC) -o par_t par_t.o $(LIBS)

par_p: par_p.o
	$(CC) -o par_p par_p.o $(LIBS)

%.o: %.c # generates the object files
	$(CC) $(CFLAGS) -c $*.c

clean: # cleans stuff
	rm -f $(TARGETS) *.o *~

.PHONY: all clean
