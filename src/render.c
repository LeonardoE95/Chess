#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "./include/game.h"
#include "./include/render.h"

// ----------------------------------------

void sdl2_c(int code) {
  if (code < 0) {
    printf("[ERROR] - SDL error: %s\n", SDL_GetError());
    exit(1);
  }

  return;
}

void *sdl2_p(void *ptr) {
  if (!ptr) {
    fprintf(stderr, "[ERROR] - SDL_ERROR: %s", SDL_GetError());
    exit(1);
  }

  return ptr;
}

void img_c(int code) {
  if (code < 0) {
    printf("[ERROR] - IMG error: %s\n", IMG_GetError());
    exit(1);
  }

  return;
}

void *img_p(void *ptr) {
  if (!ptr) {
    fprintf(stderr, "[ERROR] - IMG_ERROR: %s", IMG_GetError());
    exit(1);
  }

  return ptr;
}

// ----------------------------------------

void render_game(SDL_Renderer *renderer, const Game *game) {
  sdl2_c(SDL_SetRenderDrawColor(renderer, HEX_COLOR(BLACK)));  
  SDL_RenderClear(renderer);
  
  render_board(renderer);
  render_valid_moves(renderer, game);
  render_pieces(renderer, game);

  SDL_RenderPresent(renderer);  
}

void render_board(SDL_Renderer *renderer) {
  int counter, col;
  int colors[] = {GRID_COLOR_1, GRID_COLOR_2};
  
  for (int x = 0 ; x < BOARD_WIDTH; x++) {
    counter = x % 2;
    for (int y = 0; y < BOARD_HEIGHT; y++) {
      col = colors[counter];
      sdl2_c(SDL_SetRenderDrawColor(renderer, HEX_COLOR(col)));

      SDL_Rect rect = {
	(int) floorf(x * CELL_WIDTH),
	(int) floorf(y * CELL_HEIGHT),
	(int) floorf(CELL_WIDTH),
	(int) floorf(CELL_HEIGHT),
      };

      sdl2_c(SDL_RenderFillRect(renderer, &rect));

      counter = (counter + 1) % 2;
    }
  }
}

void render_piece(SDL_Renderer *renderer, Piece *p, int selected) {
  // is this the first time we render this piece?
  if (!p->texture) {
    p->image = img_p(IMG_Load(p->image_path));
    p->texture = SDL_CreateTextureFromSurface(renderer, p->image);
  }
  
  SDL_Rect chess_pos = {
    (int) floorf(p->pos.x * CELL_WIDTH),
    (int) floorf(p->pos.y * CELL_HEIGHT),
    (int) floorf(CELL_WIDTH),
    (int) floorf(CELL_HEIGHT),
  };

  SDL_RenderCopy(renderer, p->texture, NULL, &chess_pos);
  
  if (selected) {
    render_pos_highlight(renderer, p->pos, HEX_COLOR(HIGHLIGHT_COLOR_1));
  }
}

void render_pos_highlight(SDL_Renderer *renderer, Pos pos, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
  sdl2_c(SDL_SetRenderDrawColor(renderer, r, g, b, a));

  int coords[][4] = {
    // ----
    // top 
    {pos.x * CELL_WIDTH              , pos.y * CELL_HEIGHT,
     pos.x * CELL_WIDTH + CELL_WIDTH , pos.y * CELL_HEIGHT},
    
    {pos.x * CELL_WIDTH              , pos.y * CELL_HEIGHT + 1,
     pos.x * CELL_WIDTH + CELL_WIDTH , pos.y * CELL_HEIGHT + 1},
    
    {pos.x * CELL_WIDTH              , pos.y * CELL_HEIGHT + 2,
     pos.x * CELL_WIDTH + CELL_WIDTH , pos.y * CELL_HEIGHT + 2},

    // ----
    // bottom
    {pos.x * CELL_WIDTH              , pos.y * CELL_HEIGHT + CELL_HEIGHT,
     pos.x * CELL_WIDTH + CELL_WIDTH , pos.y * CELL_HEIGHT + CELL_HEIGHT},
    
    {pos.x * CELL_WIDTH              , pos.y * CELL_HEIGHT + CELL_HEIGHT - 1,
     pos.x * CELL_WIDTH + CELL_WIDTH , pos.y * CELL_HEIGHT + CELL_HEIGHT - 1},
    
    {pos.x * CELL_WIDTH              , pos.y * CELL_HEIGHT + CELL_HEIGHT - 2,
     pos.x * CELL_WIDTH + CELL_WIDTH , pos.y * CELL_HEIGHT + CELL_HEIGHT - 2},

    // ----
    // left
    {pos.x * CELL_WIDTH              , pos.y * CELL_HEIGHT,
     pos.x * CELL_WIDTH              , pos.y * CELL_HEIGHT + CELL_HEIGHT},
    
    {pos.x * CELL_WIDTH + 1          , pos.y * CELL_HEIGHT,
     pos.x * CELL_WIDTH + 1          , pos.y * CELL_HEIGHT + CELL_HEIGHT},
    
    {pos.x * CELL_WIDTH + 2          , pos.y * CELL_HEIGHT,
     pos.x * CELL_WIDTH + 2          , pos.y * CELL_HEIGHT + CELL_HEIGHT},

    // ----
    // right
    {pos.x * CELL_WIDTH + CELL_WIDTH     , pos.y * CELL_HEIGHT,
     pos.x * CELL_WIDTH + CELL_WIDTH     , pos.y * CELL_HEIGHT + CELL_HEIGHT},
    
    {pos.x * CELL_WIDTH + CELL_WIDTH - 1 , pos.y * CELL_HEIGHT,
     pos.x * CELL_WIDTH + CELL_WIDTH - 1 , pos.y * CELL_HEIGHT + CELL_HEIGHT},
    
    {pos.x * CELL_WIDTH + CELL_WIDTH - 2 , pos.y * CELL_HEIGHT,
     pos.x * CELL_WIDTH + CELL_WIDTH - 2 , pos.y * CELL_HEIGHT + CELL_HEIGHT},
    
  };

  for (int i = 0; i < 4*3; i++) {
    SDL_RenderDrawLine(renderer,
		       floorf(coords[i][0]), floorf(coords[i][1]),
		       floorf(coords[i][2]), floorf(coords[i][3]));
  }
}


void render_pieces(SDL_Renderer *renderer, const Game *game) {
  for (int x = 0; x < BOARD_WIDTH; x++) {
    for (int y = 0; y < BOARD_HEIGHT; y++) {
      if (game->board[x][y]) {
	int selected = game->selected_piece == game->board[x][y];
	render_piece(renderer, game->board[x][y], selected);
      }
    }
  }
}

void render_valid_moves(SDL_Renderer *renderer, const Game *game) {
  for (int i = 0; i < game->valid_moves_count; i++) {
    Pos p = game->valid_moves[i];
    render_pos_highlight(renderer, p, HEX_COLOR(HIGHLIGHT_COLOR_2));
  }
}
