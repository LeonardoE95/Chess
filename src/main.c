/*
  Dipendenze:

  - SDL2

    sudo pacman -S sdl2 (archlinux)
    sudo apt-get install libsdl2-dev (ubuntu)

  - SDL2_Image

    sudo pacman -S sdl2_image (archlinux)
    sudo apt-get install ??? (ubuntu)

 */


#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define SCREEN_WIDTH  600
#define SCREEN_HEIGHT 600

#define BOARD_WIDTH 8
#define BOARD_HEIGHT 8

#define CELL_WIDTH ((SCREEN_WIDTH / BOARD_WIDTH))
#define CELL_HEIGHT ((SCREEN_HEIGHT / BOARD_HEIGHT))

// ----------------------------------------
// R: 125, G: 148, B:  93 | Hex: #7D945D
// R: 238, G: 238, B: 213 | Hex: #EEEED5o

// RGBA, Red Green Blue Alpha
#define GRID_COLOR_1 0xEEEED500
#define GRID_COLOR_2 0x7D945D00


// Tsoding
// https://www.twitch.tv/tsoding
// https://github.com/tsoding
#define HEX_COLOR(hex)							\
  ((hex) >> (3 * 8)) & 0xFF,						\
  ((hex) >> (2 * 8)) & 0xFF,						\
  ((hex) >> (1 * 8)) & 0xFF,						\
  ((hex) >> (0 * 8)) & 0xFF


void scc(int code) {
// ----------------------------------------

typedef enum {
  B_KING = 0,
  B_QUEEN,
  B_ROOK,
  B_BISHOP,
  B_KNIGHT,
  B_PAWN,

  W_KING,
  W_QUEEN,
  W_ROOK,
  W_BISHOP,
  W_KNIGHT,
  W_PAWN,

  EMPTY,

} PieceType;

#define IS_BLACK(x) (x >= 0 && x <= 5)
#define IS_WHITE(x) (x >= 6 && x <= 11)

typedef struct {
  int x;
  int y;
} Pos;

typedef struct {
  PieceType type;
  Pos pos;

  SDL_Surface *image;
  SDL_Texture *texture;
} Piece;

typedef struct {
  Piece *board[BOARD_WIDTH][BOARD_HEIGHT];
  PieceType w_score[16];
  PieceType b_score[16];
  int quit;
} Game;

  if (code < 0) {
    printf("SDL error: %s\n", SDL_GetError());
    exit(1);
  }

  return;
}

void *scp(void *ptr) {
  if (!ptr) {
    fprintf(stderr, "[ERROR] - SDL_ERROR: %s", SDL_GetError());
    exit(1);
  }

  return ptr;
}

void render_board(SDL_Renderer *renderer) {
  int counter, col;
  int colors[] = {GRID_COLOR_1, GRID_COLOR_2};
  
  for (int x = 0 ; x < BOARD_WIDTH; x++) {
    counter = x % 2;
    for (int y = 0; y < BOARD_HEIGHT; y++) {
      col = colors[counter];
      scc(SDL_SetRenderDrawColor(renderer, HEX_COLOR(col)));

      SDL_Rect rect = {
	(int) floorf(x * CELL_WIDTH),
	(int) floorf(y * CELL_HEIGHT),
	(int) floorf(CELL_WIDTH),
	(int) floorf(CELL_HEIGHT),
      };

      scc(SDL_RenderFillRect(renderer, &rect));

      counter = (counter + 1) % 2;
    }
  }
  
}

// ----------------------------------------

int main(void) {
  // init classic SDL
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window *const window = scp(SDL_CreateWindow("Description", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE));  
  SDL_Renderer *const renderer = scp(SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED));

  // init image SDL
  IMG_Init(IMG_INIT_PNG);

  SDL_Surface *image = IMG_Load("../assets/black_king.svg");
  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, image);
  
  int quit = 0;
  
  while(!quit) {
    SDL_Event event;

    // event handling
    while(SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
	quit = 1;
      }
    }

    scc(SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255));
    SDL_RenderClear(renderer);
    render_board(renderer);

    int x = 0;
    int y = 0;
    
    SDL_Rect chess_pos = {
	(int) floorf(x * CELL_WIDTH),
	(int) floorf(y * CELL_HEIGHT),
	(int) floorf(CELL_WIDTH),
	(int) floorf(CELL_HEIGHT),
    };

    SDL_RenderCopy(renderer, texture, NULL, &chess_pos);
    
    SDL_RenderPresent(renderer);
  }

  SDL_DestroyTexture(texture);
  SDL_FreeSurface(image);
  
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  
  IMG_Quit();
  SDL_Quit();
}

