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
        true,
        NULL,
        0,
        NULL
    };

    ws.words = malloc(1*sizeof(Word));
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

void populate_grid(Grid *grid) {
    for(uint8_t y = 0; y < grid->size.y; y++) {
        for(uint8_t x = 0; x < grid->size.x; x++) {
            grid->letters[y][x] =
                (grid->casing ? 'A' : 'a') + rand()%(grid->casing ? 'Z' - 'A' : 'z' - 'a');
        }
    }

    populate_words(grid);
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

    pdf_game_page(grid, pdf);
    if(grid->answers)
        pdf_answer_page(grid, pdf);

    HPDF_SaveToFile(pdf, name);

    HPDF_Free(pdf);
}

void _pdf_restrict_rect(Grid *grid, HPDF_REAL font_size, HPDF_Rect *rect) {
    HPDF_REAL rect_size = grid->size.y * font_size;
    if(rect->top > rect_size) {
        rect->bottom = rect->top - rect_size;
    }
}

void pdf_game_page(Grid *grid, HPDF_Doc pdf) {
    // uint8_t word_offset = grid->amount/WPR * 25;
    uint8_t word_offset = 100;
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
    _pdf_restrict_rect(grid, font_size, &rect);

    HPDF_Page_Rectangle(search, rect.left-1, rect.bottom, rect.right - rect.left,
            rect.top - rect.bottom);
    HPDF_Page_Stroke(search);

    HPDF_Page_BeginText(search);

    // grid
    HPDF_Page_SetFontAndSize(search, font, font_size);
    pdf_draw_grid(search, rect, grid);

    // words
    word_offset = 10;
    rect.bottom -= word_offset;
    HPDF_Page_SetFontAndSize(search, font, word_offset);
    HPDF_TransMatrix Tm = HPDF_Page_GetTextMatrix(search);
    HPDF_Page_SetTextMatrix(search, Tm.a, Tm.b, Tm.c + .2, Tm.d, Tm.x, Tm.y);
    for(uint8_t i = 0; i < grid->amount; i++) {
        char *word = grid->words[i].word;

        HPDF_Page_TextOut(search,
                rect.left,
                rect.bottom,
                word
                );
        rect.left += (_pdf_word_width(font, word) * word_offset / 1000) + 10;
        if(rect.left >= rect.right) {
            rect.left = 25;
            rect.bottom -= word_offset + 10;
        }
    }

    HPDF_Page_EndText(search);
}

void pdf_answer_page(Grid *grid, HPDF_Doc pdf) {
    // clear grid for just answers
    for(uint8_t y = 0; y < grid->size.y; y++) {
        for(uint8_t x = 0; x < grid->size.x; x++) {
            grid->letters[y][x] = ' ';
        }
    }
    populate_words(grid);

    HPDF_Page answers = HPDF_AddPage(pdf);
    HPDF_Page_SetSize(answers, HPDF_PAGE_SIZE_A5, HPDF_PAGE_PORTRAIT);
    HPDF_Font font = HPDF_GetFont(pdf, "Helvetica", NULL);
    HPDF_Page_SetTextLeading(answers, 20);
    HPDF_REAL width = HPDF_Page_GetWidth(answers);
    HPDF_REAL height = HPDF_Page_GetHeight(answers);
    const HPDF_REAL font_size = width/grid->size.x;
    HPDF_Rect rect = {
        25,         // left
        25,         // bottom
        width - 25, // right
        height - 25,// top
    };
    _pdf_restrict_rect(grid, font_size, &rect);

    HPDF_Page_Rectangle(answers, rect.left-1, rect.bottom, rect.right - rect.left,
            rect.top - rect.bottom);
    HPDF_Page_Stroke(answers);

    HPDF_Page_BeginText(answers);

    // grid
    HPDF_Page_SetFontAndSize(answers, font, font_size);
    pdf_draw_grid(answers, rect, grid);
    HPDF_Page_EndText(answers);
}

void pdf_draw_grid(HPDF_Page page, HPDF_Rect rect, Grid *grid) {
    const HPDF_REAL font_size = HPDF_Page_GetWidth(page)/grid->size.x;
    for(uint8_t y = 0; y < grid->size.y; y++) {
        rect.left = 25 - font_size;
        for(uint8_t x = 0; x < grid->size.x; x++) {
            char *letter = calloc('\0', 2*sizeof(char));
            letter[0] = grid->letters[y][x];

            HPDF_Page_TextRect(page,
                rect.left += font_size,
                rect.top, rect.right, rect.bottom,
                letter, HPDF_TALIGN_JUSTIFY, NULL
            );
        }
        rect.top -= font_size;
    }
}

HPDF_INT _pdf_word_width(HPDF_Font font, const char *word) {
    HPDF_INT width = 0;
    for(size_t i = 0; i < strlen(word); i++)
        width += HPDF_Font_GetUnicodeWidth(font, word[i]);
    return width;
}
