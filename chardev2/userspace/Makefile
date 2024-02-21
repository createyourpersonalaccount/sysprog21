.PHONY: all clean

CFLAGS ?= -I../include

all: main

main: main.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f main
