CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -pthread

TARGETS = seq par_t par_p

all: $(TARGETS)

seq: seq.c list.h
	$(CC) $(CFLAGS) -o seq seq.c

par_t: par_t.c list.h
	$(CC) $(CFLAGS) -o par_t par_t.c $(LDFLAGS)

par_p: par_p.c list.h
	$(CC) $(CFLAGS) -o par_p par_p.c

clean:
	rm -f $(TARGETS) *.o
