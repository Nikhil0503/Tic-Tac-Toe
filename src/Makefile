CC=gcc
CFLAGS=-Wall -g -Wextra -pedantic -std=c99 -fsanitize=address,undefined

ttts: ttts.o
	$(CC) $(CFLAGS) -o $@ $^

ttts.o: ttts.c
	$(CC) $(CFLAGS) -c -o $@ $<

ttt: ttt.o
	$(CC) $(CFLAGS) -o $@ $^

ttt.o: ttt.c
	$(CC) $(CFLAGS) -c -o $@ $<
