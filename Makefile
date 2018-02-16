.PHONY: all clean install uninstall

CC     = gcc
CFLAGS = -std=c99 -Wall -Werror -Wextra
PREFIX = $(DESTDIR)/usr/local

all: fag

fag: fag.c fag.h
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f fag

install: fag
	install -D -m 0755 -t $(PREFIX)/bin/ $^

uninstall:
	rm -f $(PREFIX)/bin/fag
