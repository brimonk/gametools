CFLAGS=-Wall -g3

all: rcubestool wordle

wordle: wordle.c

.PHONY:
clean:
	rm -f rcubestool wordle

