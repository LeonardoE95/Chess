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
#include <stdlib.h>
#include <assert.h>
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

  Piece *selected_piece;
  int quit;
} Game;

void sdl2_c(int code);
void *sdl2_p(void *ptr);
void img_c(int code);
void *img_p(void *ptr);

void init_game(Game *game);
Piece *init_piece(PieceType t, Pos init_pos);

void destroy_piece(Piece *p);

void render_game(SDL_Renderer *renderer, Game *game);
void render_pieces(SDL_Renderer *renderer, Game *game);
void render_piece(SDL_Renderer *renderer, Piece *p, int selected);
void render_board(SDL_Renderer *renderer);

// ----------------------------------------

// GLOBAL VARIABLES

const char *TYPE2PNG[] = {
    [B_KING]    = "../assets/black_king.png",
    [B_QUEEN]   = "../assets/black_queen.png",
    [B_ROOK]    = "../assets/black_rook.png",
    [B_BISHOP]  = "../assets/black_bishop.png",
    [B_KNIGHT]  = "../assets/black_knight.png",
    [B_PAWN]    = "../assets/black_pawn.png",
    // ---------
    [W_KING]    = "../assets/white_king.png",
    [W_QUEEN]   = "../assets/white_queen.png",
    [W_ROOK]    = "../assets/white_rook.png",
    [W_BISHOP]  = "../assets/white_bishop.png",
    [W_KNIGHT]  = "../assets/white_knight.png",
    [W_PAWN]    = "../assets/white_pawn.png",      
};

const PieceType DEFAULT_BOARD[BOARD_HEIGHT][BOARD_WIDTH] = {
  {B_ROOK, B_KNIGHT, B_BISHOP, B_QUEEN, B_KING, B_BISHOP, B_KNIGHT, B_ROOK},
  {B_PAWN, B_PAWN  , B_PAWN  , B_PAWN , B_PAWN, B_PAWN  , B_PAWN  , B_PAWN},
  
  {EMPTY , EMPTY   , EMPTY   , EMPTY  , EMPTY , EMPTY   , EMPTY   , EMPTY},
  {EMPTY , EMPTY   , EMPTY   , EMPTY  , EMPTY , EMPTY   , EMPTY   , EMPTY},
  {EMPTY , EMPTY   , EMPTY   , EMPTY  , EMPTY , EMPTY   , EMPTY   , EMPTY},
  {EMPTY , EMPTY   , EMPTY   , EMPTY  , EMPTY , EMPTY   , EMPTY   , EMPTY},
  
  {W_PAWN, W_PAWN  , W_PAWN  , W_PAWN , W_PAWN, W_PAWN  , W_PAWN  , W_PAWN},
  {W_ROOK, W_KNIGHT, W_BISHOP, W_QUEEN, W_KING, W_BISHOP, W_KNIGHT, W_ROOK},  
};


Game GAME = {0};

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

void init_game(Game *game) {
  game->quit = 0;

  for (int x = 0; x < BOARD_WIDTH; x++) {
    for (int y = 0; y < BOARD_HEIGHT; y++) {
      // NOTE: have to swap coordinates of default board to accomodate
      // SDL2 coordinate scheme
      PieceType t = DEFAULT_BOARD[y][x];
      game->board[x][y] = t != EMPTY ? init_piece(t, (Pos){x, y}) : NULL;
    }
  }
}

void destroy_game(Game *game) {
  for (int x = 0; x < BOARD_WIDTH; x++) {
    for (int y = 0; y < BOARD_HEIGHT; y++) {
      if (game->board[x][y]) {
	destroy_piece(game->board[x][y]);
      }
    }
  }  
}


// Used to istantiate a particular chess piece depending on its type.
// NOTE: the texture instantiation is de-ferred to the first call of
// render_piece().
// NOTE: assume t != EMPTY (always)
Piece *init_piece(PieceType t, Pos init_pos) {
  assert(t != EMPTY && "Piece shouldn't be EMPTY!");
  
  Piece *p = calloc(1, sizeof(Piece));
  p->pos = init_pos;
  p->type = t;
  p->image = img_p(IMG_Load(TYPE2PNG[t]));
  
  return p;
}

void destroy_piece(Piece *p) {
  SDL_DestroyTexture(p->texture);
  SDL_FreeSurface(p->image);
  free(p);
}

void update_selected_piece(Game *game, Pos p) {
  game->selected_piece = game->board[p.x][p.y];
}

int check_move(Game *game, Piece *p, Pos new_pos) {
  // returns 1 if the piece p can move from its current position to
  // new_pos, 0 otherwise.
  Pos old_pos = p->pos;
  int valid = 0;

  int dx = new_pos.x - old_pos.x;
  int dy = new_pos.y - old_pos.y;

  switch(p->type) {
  case B_PAWN:

    // at the start the pawn can choose to move two squares below.
    if (old_pos.y == 1 && dy == 2 && dx == 0) {
      // TODO: check for collision/out of board positions
      valid = 1;
    }
    
    // in general however it can only move one square below.
    else if (dy == 1 && dx == 0) {
      // TODO: check for collision/out of board positions
      valid = 1; 
    }

    // unless its trying to eat some piece using the diagonals.
    else if (1) { 
      // TODO: implement this case
      // TODO: check for collision/out of board positions
    }
    
    break;

  case W_PAWN:

    // at the start the pawn can choose to move two squares below.
    if (old_pos.y == 6 && dy == -2 && dx == 0) {
      // TODO: check for collision/out of board positions
      valid = 1;
    }
    
    // in general however it can only move one square below.
    else if (dy == -1 && dx == 0) {
      // TODO: check for collision/out of board positions
      valid = 1;
    }

    // unless its trying to eat some piece using the diagonals.
    else if (1) { 
      // TODO: implement this case
    }
    
    break;


  case B_ROOK:
  case W_ROOK:
    if (dy == 0 || dx == 0) {
      valid = 1;
    }
    
    break;

  case B_BISHOP:
  case W_BISHOP:
    if (abs(dx) == abs(dy)) {
      valid = 1;
    }
    
    break;

  case B_KNIGHT:
  case W_KNIGHT:
    break;

  case B_QUEEN:
  case W_QUEEN:
    break;

  case B_KING:
  case W_KING:
    break;

  case EMPTY:
    break;

  default:
    fprintf(stderr, "[ERROR] - Default clause in check move!\n");
    exit(1);
    break;
  }
  
  return valid;
}

void move_piece(Game *game, Piece *p, Pos new_pos) {
  if(!check_move(&GAME, p, new_pos)) {
    return;
  }
  
  printf("valid move :D!\n");

  // are we eating something?
  if(game->board[new_pos.x][new_pos.y]) {
    // TODO: handle this case
  } else {
    game->board[p->pos.x][p->pos.y] = NULL;
    game->board[new_pos.x][new_pos.y] = p;
    p->pos = new_pos;
  }
  
}

// ----------------------------------------

void render_game(SDL_Renderer *renderer, Game *game) {
  render_board(renderer);
  render_pieces(renderer, game);
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
  // was the piece already rendered?
  if (!p->texture) {
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
    // TODO: if the piece was selected we also have to indicate it
    // somehow.
  }
  
}


void render_pieces(SDL_Renderer *renderer, Game *game) {
  for (int x = 0; x < BOARD_WIDTH; x++) {
    for (int y = 0; y < BOARD_HEIGHT; y++) {
      if (game->board[x][y]) {
	int selected = game->selected_piece == game->board[x][y];
	render_piece(renderer, game->board[x][y], selected);
      }
    }
  }
}

// ----------------------------------------

int main(void) {  
  // init classic SDL
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window *const window = sdl2_p(SDL_CreateWindow("Description", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE));  
  SDL_Renderer *const renderer = sdl2_p(SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED));

  // init image SDL
  IMG_Init(IMG_INIT_PNG);
  init_game(&GAME);

  while(!GAME.quit) {
    SDL_Event event;

    // event handling
    while(SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
	GAME.quit = 1;
      }

      if (event.type == SDL_MOUSEBUTTONDOWN) {
	Pos pos = (Pos){
	  (int) floorf(event.button.x / CELL_WIDTH),
	  floorf(event.button.y / CELL_HEIGHT)
	};

	// has the player started a movement?
	if (!GAME.selected_piece) {
	  update_selected_piece(&GAME, pos);
	  printf("[INFO] - Player has picked piece at position: (%d, %d)\n", pos.x, pos.y);
	} else {
	  Piece *p = GAME.selected_piece;
	  printf("[INFO] - Player wants to move piece %d from pos: (%d, %d) to pos (%d, %d)\n", p->type, p->pos.x, p->pos.y, pos.x, pos.y);
	  move_piece(&GAME, p, pos);
	  GAME.selected_piece = NULL;
	}
      }
      
    }

    sdl2_c(SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255));
    SDL_RenderClear(renderer);
    render_game(renderer, &GAME);
    SDL_RenderPresent(renderer);
  }

  destroy_game(&GAME);
  
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  
  IMG_Quit();
  SDL_Quit();
}

