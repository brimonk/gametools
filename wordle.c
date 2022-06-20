// Brian Chrzanowski
// 2022-06-05 02:04:34
//
// My Wordle Solver.
//
// I'd been meaning to write one of these for a while. Basically, we slurp up the entire wordlist,
// and we literally just throw away words if they don't match the Worlde word.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <ctype.h>
#include <limits.h>

#define DICTFILE "/usr/share/dict/american-english"
#define LISTLIM  (1 << 20) // seems big enough :p
#define WORDLELEN (5)
#define LETTERS 26

char *WORDLIST[LISTLIM];
int TOTALWORDS = 0;

void strtolower(char *s)
{
	for (;*s;s++) *s = tolower(*s);
}

void mktab(int *tab, char c, char *w, char *r)
{
	for (int i = 0; i < WORDLELEN; i++) {
		if (r == NULL || r[i] == c)
			tab[w[i] - 'a']++;
	}
}

int chkwon(char *s)
{
	while (*s) {
		if (*s != 'g')
			return 0;
		s++;
	}
	return 1;
}

int xmatch(char *candidate, char *word, char *result)
{
	int tab[26];

	memset(&tab, 0, sizeof tab);
	mktab(tab, 'x', word, result);

	for (int i = 0; i < WORDLELEN; i++) {
		if (tab[candidate[i] - 'a'] > 0)
			return 1;
	}

	return 0;
}

int gmatch(char *candidate, char *word, char *result)
{
	int i;

	for (i = 0; i < 5; i++)
		if (result[i] == 'g' && candidate[i] != word[i])
			return 0;

	return 1;
}

int ymatch(char *candidate, char *word, char *result)
{
	// NOTE (Brian) check if the candidate word has ALL yellow letters in a DIFFERENT spot
	//
	// - candidate    the word we're attempting to filter in / out
	// - word         the word the user input
	// - result (the result from the wordle game)

	int i, j;
	int rc;

	rc = 0;

	for (i = 0; i < WORDLELEN; i++) {
		if (result[i] != 'y')
			continue;

		if (word[i] == candidate[i]) // yellow letter in same spot on candidate
			return false;

		for (j = 0; j < WORDLELEN; j++) {
			if (i != j && candidate[j] == word[i]) {
				rc = true;
			}
		}
	}

	return rc;
}

void crunchlist(char *word, char *result)
{
	for (int i = 0; i < TOTALWORDS; i++) {
		if (WORDLIST[i] == NULL)
			continue;

		if (xmatch(WORDLIST[i], word, result) || !(gmatch(WORDLIST[i], word, result) && ymatch(WORDLIST[i], word, result))) {
			free(WORDLIST[i]);
			WORDLIST[i] = NULL;
		}
	}
}

void printentries(int *e, int n)
{
	int printed = 0;

	while ((*e) < TOTALWORDS && printed < n) {
		if (WORDLIST[*e] != NULL) {
			printf("%s\n", WORDLIST[*e]);
			printed++;
		}
		(*e)++;
	}
}

int main(int argc, char **argv)
{
	FILE *fp;
	char buf[1024];
	int i;

	memset(WORDLIST, 0, sizeof WORDLIST);

	fp = fopen(DICTFILE, "rb");
	if (!fp) {
		fprintf(stderr, "couldn't open dictionary file!\n");
		return 1;
	}

	for (i = 0; i < LISTLIM && buf == fgets(buf, sizeof buf, fp); i++) {
		buf[strlen(buf) - 1] = 0;

		if (strlen(buf) != 5) // wordle words are 5 chars long
			continue;

#define CHARTEST(x) !(isalpha((x)) && islower((x)))

		if (CHARTEST(buf[0])) continue;
		if (CHARTEST(buf[1])) continue;
		if (CHARTEST(buf[2])) continue;
		if (CHARTEST(buf[3])) continue;
		if (CHARTEST(buf[4])) continue;

		assert(i < LISTLIM);

		WORDLIST[TOTALWORDS++] = strdup(buf);
	}

	fclose(fp);

#if 0
	for (i = 0; i < TOTALWORDS; i++) {
		printf("%s\n", WORDLIST[i]);
	}
#endif

	char word[8];
	char result[8];
	int win_state, guess;
	int printidx;
	int printamt;

	// As we ask the user for guesses and board states, we begin to filter the list to appropriate
	// words that could be used in the future, assuming all input information is correct. To do
	// this, we actually just free the item in the list and set the pointer to NULL, if it violates
	// any of the information that we've got.

	printidx = 0;
	printamt = 10;

	win_state = guess = 0;

	while (!win_state) {
		printf("G > ");
		fgets(word, sizeof word, stdin);
		word[strlen(word) - 1] = 0;
		strtolower(word);

		if (strlen(word) == 0) {
			printentries(&printidx, printamt);
			continue;
		}

		printf("S > ");
		fgets(result, sizeof result, stdin);
		result[strlen(result) - 1] = 0;
		strtolower(result);

		win_state = chkwon(result);
		if (win_state)
			break;

		crunchlist(word, result);

		printidx = 0;

		printentries(&printidx, printamt); // we literally don't do anything special to print a diverse set of guesses

		guess++;
	}

	if (win_state) {
		printf("CONGRATS, YOU CHEATED!\n");
	} else {
		printf("You couldn't even win without cheating? Pathetic.\n");
	}

	return 0;
}
