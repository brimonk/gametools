CFLAGS += -Wall -g3

all: wordle

wordle: wordle.c

.PHONY:
clean:
	rm -f rcubestool wordle

