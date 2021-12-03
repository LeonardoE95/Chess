#ifndef RENDER_H_
#define RENDER_H_

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

// ----------------------------------------
// R: 125, G: 148, B:  93 | Hex: #7D945D
// R: 238, G: 238, B: 213 | Hex: #EEEED5o

// RGBA, Red Green Blue Alpha
#define BLACK             0x000000FF
#define GRID_COLOR_1      0xEEEED500
#define GRID_COLOR_2      0x7D945D00
#define HIGHLIGHT_COLOR_1 0xEE72F100
#define HIGHLIGHT_COLOR_2 0xFF8C0000

// Tsoding
// https://www.twitch.tv/tsoding
// https://github.com/tsoding
#define HEX_COLOR(hex)							\
  ((hex) >> (3 * 8)) & 0xFF,						\
  ((hex) >> (2 * 8)) & 0xFF,						\
  ((hex) >> (1 * 8)) & 0xFF,						\
  ((hex) >> (0 * 8)) & 0xFF

void sdl2_c(int code);
void *sdl2_p(void *ptr);
void img_c(int code);
void *img_p(void *ptr);

void render_game(SDL_Renderer *renderer, const Game *game);
void render_board(SDL_Renderer *renderer);
void render_pieces(SDL_Renderer *renderer, const Game *game);
void render_piece(SDL_Renderer *renderer, Piece *p, int selected);
void render_board(SDL_Renderer *renderer);
void render_pos_highlight(SDL_Renderer *renderer, Pos p, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
void render_valid_moves(SDL_Renderer *renderer, const Game *game);

#endif // RENDER_H_
