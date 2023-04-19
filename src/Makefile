CC=gcc
CFLAGS=-Wall -g -Wextra -pedantic -std=c99 -fsanitize=address,undefined -pthread
LDFLAGS= -L/lib/x86_64-linux-gnu -lasan

all: ttts ttts.o ttt ttt.o

ttts: ttts.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

ttts.o: ttts.c
	$(CC) $(CFLAGS) -c -o $@ $<

ttt: ttt.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

ttt.o: ttt.c
	$(CC) $(CFLAGS) -c -o $@ $<