// TODO: implement variations from https://en.wikipedia.org/wiki/Word_search
#include <stdint.h>
#include <stdbool.h>
#include <hpdf.h>

#define RAND_BIN (rand() > RAND_MAX/2)
#define COORD_EQ(a, b) (a.x == b.x && a.y == b.y)

typedef struct {
    uint8_t x;
    uint8_t y;
} Coord;

Coord _max_coord(Coord a, Coord b);
Coord _min_coord(Coord a, Coord b);

typedef enum {
    Vertical,
    Horizontal,
    Diagonal
} Order;

typedef struct {
    char *word;     // word string
    Coord first;    // first letters position
    Coord last;     // last letters position
} Word;

// TODO: spaces inbetween words?
typedef struct {
    Coord size;     // rectangle (width, height) of grid
    bool casing;    // 0 - lowercase, 1 - uppercase
    bool answers;
    char **letters;
    uint8_t amount; // amount of words
    Word *words;
} Grid;

void append_word(Grid *grid, char *word);
void populate_grid(Grid *grid);
void populate_words(Grid *grid);
void free_grid(Grid *grid);

void pdf_draw_grid(HPDF_Page page, HPDF_Rect rect, Grid *grid);
void pdf_game_page(Grid *grid, HPDF_Doc pdf);
void pdf_answer_page(Grid *grid, HPDF_Doc pdf);
HPDF_INT _pdf_word_width(HPDF_Font font, const char *word);
void _pdf_restrict_rect(Grid *grid, HPDF_REAL font_size, HPDF_Rect *rect);

void debug_display_grid(Grid *grid);
void pdf_export(Grid *grid, char *name);
