.PHONY: all clean

all: fag

fag:
	gcc -std=c99 -Wall -Werror -Wextra fag.c -o fag

clean:
	rm -f fag
