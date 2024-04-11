// TODO: implement variations from https://en.wikipedia.org/wiki/Word_search
#include <stdint.h>

#define RANDC (rand() > RAND_MAX/2)

typedef enum {
    Vertical,
    Horizontal,
    Diagonal
} Order;

typedef struct {
    uint8_t x;
    uint8_t y;
} Coord;

typedef struct {
    char *word; // word string
    Coord first;// first letters position
    Coord last; // last letters position
} Word;

typedef struct {
    Coord size; // rectangle (width, height) of grid
    char **letters;
    Word *words;
} Grid;

Word generate_word(char *word, Coord max);
void generate_grid(Grid *grid);

void debug_display_grid(Grid *grid);
