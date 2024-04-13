#include "word_search.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

// #include <hpdf.h>
#include "../libharu/include/hpdf.h"

int main(int argc, char *argv[]) {
    srand(time(NULL));

    Grid ws = (Grid) {
        (Coord) {100, 100},
        false,
        NULL,
        0,
        NULL
    };

    ws.words = malloc(7*sizeof(Word));
    append_word(&ws, "test");

    ws.letters = malloc(ws.size.x * ws.size.y * sizeof(char *));
    for(uint8_t x = 0; x < ws.size.x; x++)
        ws.letters[x] = malloc(ws.size.y);
    populate_grid(&ws);

    export_pdf(&ws);
    debug_display_grid(&ws);

    free_grid(&ws);
    return 0;
}

// TODO: bounds checking
void append_word(Grid *grid, char *word) {
    size_t radius = strlen(word);
    Order position = RANDC + RANDC;

    Coord first = {
        rand()%grid->size.x,
        rand()%grid->size.y
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

    grid->words[grid->amount++] = (Word) {
        word,
        first,
        last
    };
}

void populate_grid(Grid *grid) {
    for(uint8_t y = 0; y < grid->size.y; y++) {
        for(uint8_t x = 0; x < grid->size.x; x++) {
            grid->letters[y][x] = (grid->casing ? 'A' : 'a') + rand()%(grid->casing ? 'Z' - 'A' : 'z' - 'a');
        }
    }

    populate_words(grid);
}

Coord _max_coord(Coord a, Coord b) {
    return (Coord) {
        fmax(a.x, b.x),
        fmax(a.y, b.y)
    };
}

Coord _min_coord(Coord a, Coord b) {
    return (Coord) {
        fmin(a.x, b.x),
        fmin(a.y, b.y)
    };
}

void populate_words(Grid *grid) {
    for(uint8_t i = 0; i < grid->amount; i++) {
        Word word = grid->words[i];
        Coord first = _min_coord(word.first, word.last);
        Coord last = _max_coord(word.first, word.last);
        const uint8_t length = strlen(word.word) - 1;
        const bool forwards = word.first.x <= word.last.x;

        uint8_t y = first.y;
        const bool diagonal = word.first.y != word.last.y;
        const bool direction = word.first.y > word.last.y;
        for(uint8_t x = first.x; x < last.x; x++) {
            char letter = word.word[(forwards ? x - first.x : length - (x - first.x))];
            grid->letters[y += diagonal ? direction*2-1 : 0][x] = grid->casing ? toupper(letter) : tolower(letter);
        }
    }
}

void debug_display_grid(Grid *grid) {
    for(uint8_t i = 0; i < grid->amount; i++)
        printf("\"%s\" {(%d, %d), (%d, %d)}\n", grid->words[i].word, grid->words[i].first.x, grid->words[i].first.y, grid->words[i].last.x, grid->words[i].last.y);
    for(uint8_t y = 0; y < grid->size.y; y++) {
        for(uint8_t x = 0; x < grid->size.x; x++) {
            printf("%c", grid->letters[y][x]);
        }
        printf("\n");
    }
}

void free_grid(Grid *grid) {
    for(uint8_t x = 0; x < grid->size.x; x++)
        free(grid->letters[x]);
    free(grid->letters);
    free(grid->words);
}

void export_pdf(Grid *grid) {
    HPDF_Doc pdf = HPDF_New(
        (HPDF_Error_Handler) {
            0, 0, NULL
        },
        NULL
    );
}
