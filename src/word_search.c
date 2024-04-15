#include "word_search.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#include <hpdf.h>
#include "../libharu/include/hpdf.h" // for lsp

int main(int argc, char *argv[]) {
    srand(time(NULL));

    Grid ws = (Grid) {
        (Coord) {100, 100},
        true,
        false,
        NULL,
        0,
        NULL
    };

    ws.words = malloc(2*sizeof(Word));
    append_word(&ws, "test");
    append_word(&ws, "test");

    ws.letters = malloc(ws.size.x * ws.size.y * sizeof(char *));
    for(uint8_t x = 0; x < ws.size.x; x++)
        ws.letters[x] = malloc(ws.size.y);
    populate_grid(&ws);

    pdf_export(&ws, argv[1]);
    // debug_display_grid(&ws);

    free_grid(&ws);
    return 0;
}

// TODO: bounds checking
void append_word(Grid *grid, char *word) {
    size_t radius = strlen(word);
    Order position = RAND_BIN + RAND_BIN;

    Coord first = {
        rand()%grid->size.x,
        rand()%grid->size.y
    };

    Coord last = first;
    switch(position) {
        case Vertical:
            last.y += radius * (RAND_BIN*2 - 1);
        case Horizontal:
            last.x += radius * (RAND_BIN*2 - 1);
            break;
        case Diagonal:
            last.x += radius * (RAND_BIN*2 - 1);
            last.y += radius * (RAND_BIN*2 - 1);
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
            grid->letters[y][x] = grid->answers ? '.' :
                (grid->casing ? 'A' : 'a') + rand()%(grid->casing ? 'Z' - 'A' : 'z' - 'a');
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

void pdf_export(Grid *grid, char *name) {
    HPDF_Doc pdf = HPDF_New(
        (HPDF_Error_Handler) {
            0, 0, NULL
        },
        NULL
    );

    if(!pdf) {
        fprintf(stderr, "failed to create pdf object\n");
        return;
    }

    HPDF_SetCompressionMode(pdf, HPDF_COMP_ALL);
    uint8_t word_offset = grid->amount/WPR * 25;
    // page one
    {
        HPDF_Page search = HPDF_AddPage(pdf);
        HPDF_Page_SetSize(search, HPDF_PAGE_SIZE_A5, HPDF_PAGE_PORTRAIT);
        HPDF_Font font = HPDF_GetFont(pdf, "Helvetica", NULL);
        HPDF_Page_SetTextLeading(search, 20);
        HPDF_REAL width = HPDF_Page_GetWidth(search);
        HPDF_REAL height = HPDF_Page_GetHeight(search);
        const HPDF_REAL font_size = width/grid->size.x;
        HPDF_Rect rect = {
            25,                 // left
            25 + word_offset,   // bottom
            width - 25,         // right
            height - 25,        // top
        };

        HPDF_Page_Rectangle(search, rect.left-1, rect.bottom, rect.right - rect.left,
                rect.top - rect.bottom);
        HPDF_Page_Stroke(search);

        HPDF_Page_BeginText(search);

        // grid
        HPDF_Page_SetFontAndSize(search, font, font_size);
        for(uint8_t y = 0; y < grid->size.y; y++) {
            for(uint8_t x = 0; x < grid->size.x; x++) {
                char *letter = calloc('\0', 2*sizeof(char));
                letter[0] = grid->letters[y][x];

                HPDF_Page_TextRect(search,
                    rect.left += font_size, //HPDF_Font_GetUnicodeWidth(font, letter[0]) * font_size / 1000,
                    rect.top, rect.right, rect.bottom,
                    letter, HPDF_TALIGN_JUSTIFY, NULL
                );
            }
            rect.left = 25;
            rect.top -= font_size;
        }

        // words
        word_offset = 10;
        HPDF_Page_SetFontAndSize(search, font, word_offset);
        for(uint8_t i = 0; i < grid->amount; i++) {
            const char *word = grid->words[i].word;
            if(i%WPR == 0)
                rect.left = 25;

            HPDF_Page_TextOut(search,
                rect.left += word_offset*strlen(word),
                rect.bottom -= (i%WPR == 0) * 25,
                word
            );
        }

        HPDF_Page_EndText(search);
    }

    // page 2
    {
        grid->answers = true;
        populate_grid(grid);
        HPDF_Page answers = HPDF_AddPage(pdf);
        HPDF_Page_SetSize(answers, HPDF_PAGE_SIZE_A5, HPDF_PAGE_PORTRAIT);
        HPDF_Font font = HPDF_GetFont(pdf, "Helvetica", NULL);
        HPDF_Page_SetTextLeading(answers, 20);
        HPDF_REAL width = HPDF_Page_GetWidth(answers);
        HPDF_REAL height = HPDF_Page_GetHeight(answers);
        const HPDF_REAL font_size = width/grid->size.x;
        HPDF_Rect rect = {
            25,                 // left
            25 + word_offset,   // bottom
            width - 25,         // right
            height - 25,        // top
        };

        HPDF_Page_Rectangle(answers, rect.left-1, rect.bottom, rect.right - rect.left,
                rect.top - rect.bottom);
        HPDF_Page_Stroke(answers);

        HPDF_Page_BeginText(answers);

        // grid
        HPDF_Page_SetFontAndSize(answers, font, font_size);
        for(uint8_t y = 0; y < grid->size.y; y++) {
                HPDF_Page_TextRect(answers, rect.left, rect.top, rect.right, rect.bottom,
                        grid->letters[y], HPDF_TALIGN_JUSTIFY, NULL);
                rect.top -= font_size;
        }
        HPDF_Page_EndText(answers);
    }

    HPDF_SaveToFile(pdf, name);

    HPDF_Free(pdf);
}
