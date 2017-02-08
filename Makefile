.PHONY: all clean

all: fag

fag: fag.c fag.h
	gcc -std=c99 -Wall -Werror -Wextra fag.c -o fag

clean:
	rm -f fag
