#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// NOTE
//
// Just a small program to calculate the correct charges / firing angle for
//
// ------- Iron Nest: Heavy Turret Simulator.

int main(int argc, char **argv)
{
    if (argc == 1) {
        fprintf(stderr, "USAGE: %s <distance> [charges:distance]\n", argv[0]);
    } else {
        for (int i = 1; i < argc; i++) {
            char local[128] = { 0 };
            char *strs[8] = { 0 };

            strncpy(local, argv[i], sizeof local);

            int j = 0;
            for (char *s = strtok(local, ":"); s; s = strtok(NULL, ":")) {
                strs[j++] = s;
            }

            if (j == 1) {
                int distance = atoi(strs[0]);
                int required_charges = (distance / 5000) + 1;
                float angle = distance * 0.012f / required_charges;

                printf("D: %d, C: %d, Elevation: %2.2f\n", distance, required_charges, angle);
            } else {
                int charges = atoi(strs[0]);
                int distance = atoi(strs[1]);
                float angle = distance * 0.012f / charges;

                printf("D: %d, C: %d, Elevation: %2.2f\n", distance, charges, angle);
            }
        }
    }
}
