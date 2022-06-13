CFLAGS=-Wall -g3

all: rcubestool wordle

rcubestool: rcubestool.c

wordle: wordle.c

.PHONY:
clean:
	rm -f rcubestool wordle

