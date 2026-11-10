CFLAGS += -Wall -g3

all: wordle letter_boxed ironnest

ironnest: ironnest.c

wordle: wordle.c

letter_boxed: letter_boxed.c

.PHONY:
clean:
	rm -f letter_boxed wordle ironnest
