// Brian Chrzanowski
// 2024-01-16 00:22:33
//
// My "Letter Boxed" Solver

#include "common.h"

#define DICTFILE "/usr/share/dict/american-english"

char **DICTIONARY = NULL;
i64 **LINKS = NULL;

#define SIDES        4
#define SIDE_LENGTH  3
#define MAX_SOLVE_WORDS 32

typedef int Direction;
enum { // matches the order that we read in grid sides
    DIR_N,
    DIR_W,
    DIR_E,
    DIR_S,
};

typedef char Grid[SIDES][SIDE_LENGTH];

// NOTE If you can't solve the puzzle within 32 words, this starting word ain't it.
typedef struct PuzzleState {
    Grid *grid;
    i32 words[MAX_SOLVE_WORDS];
    i32 is_invalid;
    i32 curr;
} PuzzleState;

void init_puzzle_state(PuzzleState *puzzle_state, Grid *grid)
{
    memset(puzzle_state, 0, sizeof(*puzzle_state));
    puzzle_state->grid = grid;
    for (i32 i = 0; i < ARRSIZE(puzzle_state->words); i++) {
        puzzle_state->words[i] = -1;
    }
}

int get_grid_letter_pos(Grid *grid, char c, int *dir, int *pos)
{
    c = toupper(c);
    for (i32 i = 0; i < SIDES; i++) {
        for (i32 j = 0; j < SIDE_LENGTH; j++) {
            if (toupper((*grid)[i][j]) == c) {
                if (dir) *dir = i;
                if (pos) *pos = j;
                return true;
            }
        }
    }
    return false;
}

int letter_in_grid(Grid *grid, char c)
{
    return get_grid_letter_pos(grid, c, NULL, NULL);
}

int include_word(char *s, Grid *grid)
{
    if (strlen(s) < 3)
        return false;
    for (i32 i = 0; i < strlen(s); i++) {
        if (isblank(s[i]) || ispunct(s[i]))
            return false;
        if ((s[i] & 0x7f) != s[i])
            return false;
        if (!letter_in_grid(grid, s[i]))
            return false;
    }

    // Before we return 'true', and dictate that this word is suitable for the problem, we should
    // check to see that this word can be spelled with the given grid.

    int pdir = -1;
    int ppos = -1;

    for (char *t = s; *t; t++) {
        int cdir = -1;
        int cpos = -1;

        if (get_grid_letter_pos(grid, *t, &cdir, &cpos)) {
            if (pdir == cdir) {
                ERR("Kicking out: (%d - %d, idx: %ld) %s", pdir, cdir, t - s, s);
                ERR("  [%d][%d] == [%d][%d]", pdir, ppos, cdir, cpos);
                return false;
            }
            pdir = cdir;
            ppos = cpos;
        } else {
            return false; // How did we get here?
        }
    }

    return true;
}

int can_solving_continue(PuzzleState *state)
{
    int rc = state->words[MAX_SOLVE_WORDS - 1] == -1;
    if (!rc) state->is_invalid = true;
    return rc;
}

int is_puzzle_solved(PuzzleState *state)
{
    i32 slots[12] = {0};

    for (i64 i = 0; state->words[i] != -1; i++) {
        int dir = -1;
        int pos = -1;

        char *word = DICTIONARY[state->words[i]];

        for (char *s = word; *s; s++) {
            get_grid_letter_pos(state->grid, *s, &dir, &pos);
            slots[dir * 3 + pos]++;
        }
    }

    for (i32 i = 0; i < ARRSIZE(slots); i++) {
        if (slots[i] == 0)
            return false;
    }

    return true;
}

i64 how_many_more_letters(PuzzleState *state, i32 slots[12], char *word)
{
    i64 more = 0;

    for (char *s = word; *s; s++) {
        int dir = -1;
        int pos = -1;

        get_grid_letter_pos(state->grid, *s, &dir, &pos);
        if (slots[dir * 3 + pos] == 0)
            more++;
        slots[dir * 3 + pos]++;
    }

    return more;
}

i64 pick_next_best_word(PuzzleState *state)
{
    // TODO For this part of the algorithm, we need to have some mechanisms to look at the current
    // puzzle state and determine "how many more letters will be used by this word".

    i32 slots[12] = {0};

    for (i64 i = 0; state->words[i] != -1; i++) {
        int dir = -1;
        int pos = -1;

        char *word = DICTIONARY[state->words[i]];

        for (char *s = word; *s; s++) {
            get_grid_letter_pos(state->grid, *s, &dir, &pos);
            slots[dir * 3 + pos]++;
        }
    }

    i64 max = -1;
    i64 max_i = -1;

    for (i64 i = 0; i < arrlen(LINKS[state->curr]); i++) {
        char *word = DICTIONARY[LINKS[state->curr][i]];
        i64 v = how_many_more_letters(state, slots, word);
        if (v > max) {
            max = v;
            max_i = i;
        }
    }

    return max_i;
}

int main(int argc, char **argv)
{
    // read the dictionary first
    FILE *fp = fopen(DICTFILE, "rb");
    if (fp == NULL) {
        ERR("Couldn't open dictionary file!");
        return 1;
    }

    // Now we have to ask the user to enter in the grid. The grid is a 3x3x3x3 box, so we really
    // just have to ask people for 12 characters.
    //
    // I suggest doing it in the order: top, left, right, bottom. Comma separation will help users
    // menally see the separation.

    Grid grid = {0};

    for (;;) {
        puts("Enter the grid in the order: top, left, right, bottom, separated with commas as shown.");
        puts("Ex: \"ABC,DEF,GHI,JKL\"");

        char grid_input[128] = {0};
        bfgets(grid_input, sizeof grid_input, stdin);

        memset(&grid, 0, sizeof grid);

        i32 i = 0, j = 0;
        for (char *s = grid_input; *s; s++) {
            if (isspace(*s)) {
                continue;
            } else if (*s == ',') {
                i++;
                j = 0;
            } else {
                grid[i][j++] = toupper(*s);
            }
        }

        if (i == 3 && j == 3) // We need a better terminating condition for this.
            break;

        puts("There was an error reading the input! Please try again.");
    }

    DBG("T: %c%c%c", grid[0][0], grid[0][1], grid[0][2]);
    DBG("L: %c%c%c", grid[1][0], grid[1][1], grid[1][2]);
    DBG("R: %c%c%c", grid[2][0], grid[2][1], grid[2][2]);
    DBG("B: %c%c%c", grid[3][0], grid[3][1], grid[3][2]);

    char buf[1024];
    while (buf == bfgets(buf, sizeof buf, fp)) {
        stoupper(buf);
        if (include_word(buf, &grid)) {
            arrput(DICTIONARY, strdup(buf));
        }
    }

    fclose(fp);

    // TODO Optimize the dictionary and only consider words with letters in the grid.

    // One sub-problem is tying words with letters that end to the start of the next words, so
    // that we can quickly scan through the possibilities and determine the shortest path that
    // satisfies the condition.
    //
    // I haven't even talked about the actual problem yet! Maybe I'll keep this comment here for
    // posterity's sake, just in case anyone ever reads this code and goes "what the fuck is
    // going on?"
    //
    // Letter Boxed is a "New York Times" word puzzle game where a player has to create words
    // from the provided square. The goal is to use the fewest words that uses all of the letters
    // in the grid. The catch is that you can't use letters on the same side of the box twice in
    // a row (you have to zig-zag across the box to make words). Another catch is that the "next"
    // word MUST start with the last letter of the previous word.
    //
    // Given these rules, a brute-ish force solution with a greedy optimization is pretty simple.
    //
    // Given this, our general strategy is as follows:
    // + For all of the valid words in our dictionary (filtered from grid letters)
    // + Link each word to all possible "next" words
    // - For each word, iterate through its set of links to find the "minimum" words required to
    // solve the puzzle
    // - Find the lowest required links
    // - Print

    // Create the LINKS list. Each index of the LINKS list corresponds to the DICTIONARY entry that
    // it represents, and the elements of THAT list (child list) are indices into the DICTIONARY,
    // which have a starting letter of the final letter.

    for (i64 i = 0; i < arrlen(DICTIONARY); i++) {
        char *word = DICTIONARY[i];
        char last = word[strlen(word) - 1];
        arrput(LINKS, NULL);

        for (i64 j = 0; j < arrlen(DICTIONARY); j++) {
            if (i == j)
                continue;

            if (last == DICTIONARY[j][0]) {
                arrput(LINKS[i], j);
            }
        }
    }

    // Now, we create puzzle state for every single word in the dictionary...

    PuzzleState *puzzle_states = calloc(arrlen(DICTIONARY), sizeof(*puzzle_states));
    for (i64 i = 0; i < arrlen(DICTIONARY); i++) {
        init_puzzle_state(&puzzle_states[i], &grid);
    }

    // And now that we have puzzle state, for each puzzle state, we attempt to solve the puzzle
    // starting with that word.

    for (i64 i = 0; i < arrlen(DICTIONARY); i++) {
        PuzzleState *state = &puzzle_states[i];
        state->words[0] = i;

        // While we haven't exhausted the possible slots for this puzzle state, and the puzzle isn't
        // solved, pick the link that uses the most unused letters, and continue until the puzzle
        // is solved for this starting word.

        for (i32 j = 1; can_solving_continue(state) && !is_puzzle_solved(state); j++) {
            state->curr = j;
            i64 next_word = pick_next_best_word(state);
            state->words[j] = next_word;
        }
    }

    // PRINT OUT THE FIRST SET
    {
        PuzzleState *state = &puzzle_states[0];

        puts("A SOLUTION");

        for (i32 i = 0; state->words[i] != -1; i++) {
            printf("  %d - %s\n", i, DICTIONARY[state->words[i]]);
        }
    }

#if 0
    LOG("WRITING LINKS!");

    for (i64 i = 0; i < arrlen(LINKS); i++) {
        LOG("%s", DICTIONARY[i]);
        for (i64 j = 0; j < arrlen(LINKS[i]); j++) {
            LOG("  -> %s", DICTIONARY[LINKS[i][j]]);
        }
    }

    for (i64 i = 0; i < arrlen(DICTIONARY); i++)
        LOG("%ld - %s", i, DICTIONARY[i]);
#endif

    free(puzzle_states);
    for (i64 i = 0; i < arrlen(DICTIONARY); i++) {
        free(DICTIONARY[i]);
        arrfree(LINKS[i]);
    }
    arrfree(DICTIONARY);
    arrfree(LINKS);

    return 0;
}
