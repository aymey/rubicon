// TODO: implement variations from https://en.wikipedia.org/wiki/Word_search
#include <stdint.h>
#include <stdbool.h>

#define RANDC (rand() > RAND_MAX/2)
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
    bool casing;    // 0 - lower, 1 - upper
    char **letters;
    uint8_t amount; // amount of words
    Word *words;
} Grid;

void append_word(Grid *grid, char *word);
void populate_grid(Grid *grid);
void populate_words(Grid *grid);

void free_grid(Grid *grid);

void export_pdf(Grid *grid);
void debug_display_grid(Grid *grid);
