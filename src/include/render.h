#ifndef RENDER_H_
#define RENDER_H_

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

void sdl2_c(int code);
void *sdl2_p(void *ptr);
void img_c(int code);
void *img_p(void *ptr);

void render_game(SDL_Renderer *renderer, const Game *game);
void render_board(SDL_Renderer *renderer);
void render_pieces(SDL_Renderer *renderer, const Game *game);
void render_piece(SDL_Renderer *renderer, Piece *p, int selected);
void render_board(SDL_Renderer *renderer);
void render_pos_highlight(SDL_Renderer *renderer, Pos p);

#endif // RENDER_H_
