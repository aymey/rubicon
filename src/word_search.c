#include "word_search.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

int main(int argc, char *argv[]) {
    Grid ws = (Grid) {
        (Coord) {10, 10},
        NULL,
        NULL
    };
    ws.words[0] = generate_word("test", ws.size);

    return 0;
}

// TODO: bounds checking
Word generate_word(char *word, Coord max) {
    srand(time(NULL));

    size_t radius = strlen(word);
    Order position = RANDC + RANDC;

    Coord first = {
        rand()%max.x,
        rand()%max.y
    };

    Coord last = first;
    switch(position) {
        case Vertical:
            last.y += radius * (RANDC ? -1 : 1);
        case Horizontal:
            last.x += radius * (RANDC ? -1 : 1);
            break;
        case Diagonal:
            last.x += radius * (RANDC ? -1 : 1);
            last.y += radius * (RANDC ? -1 : 1);
        default:
            break;
    };

    return (Word) {
        word,
        first,
        last
    };
}

void generate_grid(Grid *grid) {
    for(uint8_t y = 0; y < grid->size.y; y++) {
        for(uint8_t x = 0; x < grid->size.x; x++) {
            grid->letters[y][x] = 'a';
        }
    }
}

void debug_display_grid(Grid *grid) {
    for(uint8_t y = 0; y < grid->size.y; y++) {
        for(uint8_t x = 0; x < grid->size.x; x++) {
            printf("%c", grid->letters[y][x]);
        }
        printf("\n");
    }
}
