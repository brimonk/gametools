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

#if VERBOSE_LOG
#define ERR(FMT,...)     printf("ERR: "FMT, ##__VA_ARGS__)
#define WRN(FMT,...)     printf("WRN: "FMT, ##__VA_ARGS__)
#define LOG(FMT,...)     printf("LOG: "FMT, ##__VA_ARGS__)
#define DBG(FMT,...)     printf("DBG: "FMT, ##__VA_ARGS__)
#else
#define ERR(FMT,...)
#define WRN(FMT,...)
#define LOG(FMT,...)
#define DBG(FMT,...)
#endif

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

// chkwon: returns true if every character in the string is 'g'
int chkwon(char *s)
{
	while (*s) {
		if (*s != 'g')
			return 0;
		s++;
	}
	return 1;
}

// xmatch: returns -1 if there aren't any 'x's, 0 if the word isn't a match, and 1 if it is
int xmatch(char *candidate, char *word, char *result)
{
	int tab[26];
	int grays;

	for (int i = 0; i < WORDLELEN; i++)
		if (result[i] == 'x')
			grays++;

	if (grays == 0)
		return -1;

	memset(&tab, 0, sizeof tab);

	mktab(tab, 'x', word, result);

	for (int i = 0; i < WORDLELEN; i++)
		if (tab[candidate[i] - 'a'] > 0)
			return 1;

	return 0;
}

// gmatch: returns -1 if there aren't any 'g's, 0 if there is a 'g', but it doesn't match thecandidate, and 1 if all of the 'g's match
int gmatch(char *candidate, char *word, char *result)
{
	int i, greens;

	for (i = 0, greens = 0; i < WORDLELEN; i++)
		if (result[i] == 'g')
			greens++;

	if (greens == 0)
		return -1;

	for (i = 0; i < WORDLELEN; i++)
		if (result[i] == 'g')
			if (candidate[i] != word[i])
				return 0;

	return 1;
}

// ymatch:
int ymatch(char *candidate, char *word, char *result)
{
	// NOTE (Brian) check if the candidate word has ALL yellow letters in a DIFFERENT spot
	//
	// - candidate    the word we're attempting to filter in / out
	// - word         the word the user input
	// - result (the result from the wordle game)

	int i, j, t;
	int rc;
	int yellows;

	rc = 0;

	for (i = 0, yellows = 0; i < WORDLELEN; i++)
		if (result[i] == 'y')
			yellows++;

	if (yellows == 0)
		return -1;

	// NOTE (Brian) this is in a nutty n^2 loop because we need to ensure that we account for
	// multiple yellow letters.

	for (i = 0; i < WORDLELEN; i++) {
		if (result[i] != 'y')
			continue;

		if (word[i] == candidate[i]) // yellow letter in same spot on candidate
			return 0;

		// Here, we use 't' to ensure that we find a match for EACH yellow letter.
		t = 0;

		// check if our yellow letter in the candidate matches a different spot
		for (j = 0; j < WORDLELEN; j++) {
			if (i != j && candidate[j] == word[i]) {
				rc = 1;
				t = 1;
			}
		}

		// if we didn't find a yellow letter
		if (!t) {
			return 0;
		}
	}

	return rc;
}

// crunchlist: takes the word and the result and removes words that don't match
void crunchlist(char *word, char *result)
{
	int rc;

	for (int i = 0; i < TOTALWORDS; i++) {
		if (WORDLIST[i] == NULL)
			continue;

		rc = xmatch(WORDLIST[i], word, result);
		if (rc == 1) {
			DBG("XMATCH\n");
			goto CRUNCHLIST_RELEASE;
		}

		rc = gmatch(WORDLIST[i], word, result);
		if (rc == 0) {
			DBG("GMATCH\n");
			goto CRUNCHLIST_RELEASE;
		}

		rc = ymatch(WORDLIST[i], word, result);
		if (rc == 0) {
			DBG("YMATCH\n");
			goto CRUNCHLIST_RELEASE;
		}

		continue;

CRUNCHLIST_RELEASE:

		DBG("RELEASE [%s], [%s] - [%s]\n", WORDLIST[i], word, result);
		
		free(WORDLIST[i]);

		WORDLIST[i] = NULL;
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
		memset(word, 0, sizeof word);

		do {
			printf("G > ");
			fgets(word, sizeof word, stdin);
			if (word[strlen(word) - 1] == '\n')
				word[strlen(word) - 1] = 0;
			if (strlen(word) == 0)
				printentries(&printidx, printamt);
			strtolower(word);
		} while (strlen(word) == 0);

		memset(result, 0, sizeof result);

		do {
			printf("S > ");
			fgets(result, sizeof result, stdin);
			if (result[strlen(result) - 1])
				result[strlen(result) - 1] = 0;
			strtolower(result);
		} while (strlen(result) == 0);

		win_state = chkwon(result);
		if (win_state)
			break;

		crunchlist(word, result);

		printidx = 0;

		printentries(&printidx, printamt); // we literally don't do anything special to print a diverse set of guesses

		guess++;
	}

	return 0;
}
