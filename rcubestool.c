// Brian Chrzanowski
// 2022-06-20 04:01:24
//
// WORK IN PROGRESS - DOESN'T DO ANYTHING USEFUL (yet?)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <unistd.h>

#define LO      (0)
#define HI      (114)
#define PLAYERS (2)
#define MINROLL (1)
#define MAXROLL (12)

const char *FILENAME;

struct record {
	int8_t scores[PLAYERS]; // with PLAYERS=6, it fits in 16 bytes
	int8_t turn;
	int8_t roll;
	int32_t flags;
	int64_t dist; // "hops to a winning number"
	int64_t next; // "hops to a winning number"
};

void recordprint(struct record *r)
{
	for (int i = 0; i < PLAYERS; i++)
		printf("p%d %d, ", i, r->scores[i]);
	printf("turn %d, roll %d\n", r->turn, r->roll);
}

// recordcmp: find a record with 
int recordcmp(const void *a, const void *b)
{
	struct record *key = (struct record *)a;
	struct record *mem = (struct record *)b;

	// recordprint(key);
	// recordprint(mem);

#define CMPVAL(a,b) \
	if ((a) < (b)) { return -1; } else if ((a) > (b)) { return 1; }

	// perform the most heavy handed compare function you've ever seen
	for (int i = 0; i < PLAYERS; i++) {
		CMPVAL(key->scores[i], mem->scores[i]);
	}

	CMPVAL(key->turn, mem->turn);
	CMPVAL(key->roll, mem->roll);
#undef CMPVAL

	return 0;
}

// generate: generate the table
int generate()
{
	// NOTE (Brian) If someone reading this has a better way to generate the entire table, I'd love
	// to hear it. Right now, we basically just have a single record that we use to enumerate all of
	// the values we give a shit about, then we dump it out.

	FILE *fp = fopen(FILENAME, "wb");
	if (!fp) {
		return -1;
	}

	struct record r;

	memset(&r, 0, sizeof r);

	// printf("sizeof(struct record) = %d\n", sizeof(struct record));

	// and now, for the gross part

#define FOREACH(x,lo,hi) for (x = lo; x < hi; x++)
	FOREACH(r.scores[0], LO, HI)
	FOREACH(r.scores[1], LO, HI)
	// FOREACH(r.scores[2], LO, HI)
	// FOREACH(r.scores[3], LO, HI)
	// FOREACH(r.scores[4], LO, HI)
	// FOREACH(r.scores[5], LO, HI)
	// FOREACH(r.scores[6], LO, HI)
	// FOREACH(r.scores[7], LO, HI)
	FOREACH(r.turn, 0, PLAYERS) {

		FOREACH(r.roll, -MAXROLL, -MINROLL) {
			fwrite(&r, sizeof r, 1, fp);
		}

		FOREACH(r.roll, MINROLL, MAXROLL) {
			fwrite(&r, sizeof r, 1, fp);
		}
	}
#undef FOREACH

	fclose(fp);

	return 0;
}

// lookups: perform table lookups
int lookups()
{
	return 0;
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		// usage
		return 1;
	}

	FILENAME = argv[1];

	if (access(FILENAME, F_OK) != 0) {
		generate();
	}

	lookups();

	struct record *p, srch;

	memset(&srch, 0, sizeof srch);

	srch.scores[0] = 50;
	srch.scores[1] = 32;
	srch.turn = 0;
	srch.roll = 8;

	FILE *fp = fopen(FILENAME, "rb");
	fseek(fp, -1, SEEK_END);
	size_t sz = ftell(fp);
	size_t cnt = sz / sizeof(struct record);
	rewind(fp);
	void *data = malloc(sz);
	fread(data, sizeof(struct record), cnt, fp);
	fclose(fp);

	qsort(data, cnt, sizeof(struct record), recordcmp);

	p = data;

	for (int i = 0; i < cnt; i++) {
		recordprint(p + i);
	}

	p = bsearch(&srch, data, cnt, sizeof(struct record), recordcmp);
	if (p != NULL) {
		printf("we found the record!\n");
	} else {
		printf("record not found\n");
	}

	free(data);

	return 0;
}
