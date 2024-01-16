CFLAGS += -Wall -g3

all: wordle letter_boxed

wordle: wordle.c

letter_boxed: letter_boxed.c

.PHONY:
clean:
	rm -f letter_boxed wordle
