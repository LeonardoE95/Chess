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
#define GRID_COLOR_1     0xEEEED500
#define GRID_COLOR_2     0x7D945D00
#define HIGHLIGHT_COLOR  0xEE72F100


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

typedef enum {
  UP = 0,
  DOWN,
  LEFT,
  RIGHT,
  
  DIAG_LU,
  DIAG_LD,
  DIAG_RU,
  DIAG_RD,
} Dir;

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
  PieceType score[16];
  int score_count;
} Player;

typedef struct {
  Piece *board[BOARD_WIDTH][BOARD_HEIGHT];

  Player b_player;
  Player w_player;

  Piece *selected_piece;
  Player *selected_player;
  
  int quit;
} Game;

#define IS_PIECE_BLACK(x) (x >= 0 && x <= 5)
#define IS_PIECE_WHITE(x) (x >= 6 && x <= 11)

#define IS_PLAYER_BLACK(g) (g->selected_player == &g->b_player)
#define IS_PLAYER_WHITE(g) (g->selected_player == &g->w_player)

#define SAME_TYPE(p1, p2) ((IS_PIECE_BLACK(p1->type) && IS_PIECE_BLACK(p2->type)) || (IS_PIECE_WHITE(p1->type) && IS_PIECE_WHITE(p2->type)))

void sdl2_c(int code);
void *sdl2_p(void *ptr);
void img_c(int code);
void *img_p(void *ptr);

const char *type2png(PieceType t);
void init_game(Game *game);
void destroy_game(Game *game);

Piece *init_piece(PieceType t, Pos init_pos);
void update_selected_piece(Game *game, Pos p);
void destroy_piece(Piece *p);

int check_move_validity(Game *game, Piece *p, Pos new_pos);
void move_piece(Game *game, Piece *p, Pos new_pos);
Dir compute_movement_dir(Pos start_pos, Pos end_pos);
int check_obstacles_in_path(Game *game, Pos start_pos, Pos end_pos, Dir dir);

void update_player_score(Player *p, PieceType t);

void render_game(SDL_Renderer *renderer, Game *game);
void render_pieces(SDL_Renderer *renderer, Game *game);
void render_piece(SDL_Renderer *renderer, Piece *p, int selected);
void render_board(SDL_Renderer *renderer);
void render_pos_highlight(SDL_Renderer *renderer, Pos p);

// ----------------------------------------
// GLOBAL VARIABLES

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

const char *type2png(PieceType t) {
  switch(t) {
  case B_KING:   return "../assets/black_king.png";
  case B_QUEEN:  return "../assets/black_queen.png";
  case B_ROOK:   return "../assets/black_rook.png";
  case B_BISHOP: return "../assets/black_bishop.png";
  case B_KNIGHT: return "../assets/black_knight.png";
  case B_PAWN:   return "../assets/black_pawn.png";
  // ----------------
  case W_KING:   return "../assets/white_king.png";
  case W_QUEEN:  return "../assets/white_queen.png";
  case W_ROOK:   return "../assets/white_rook.png";
  case W_BISHOP: return "../assets/white_bishop.png";
  case W_KNIGHT: return "../assets/white_knight.png";
  case W_PAWN:   return "../assets/white_pawn.png";    
    
  default:
    fprintf(stderr, "[ERROR] - default case in type2png\n");
    return "";
  }
}

void init_game(Game *game) {
  game->quit = 0;

  // init board logical state
  for (int x = 0; x < BOARD_WIDTH; x++) {
    for (int y = 0; y < BOARD_HEIGHT; y++) {
      // NOTE: swap coords to follow SDL2 coord scheme
      PieceType t = DEFAULT_BOARD[y][x];
      game->board[x][y] = t != EMPTY ? init_piece(t, (Pos){x, y}) : NULL;
    }
  }

  // NOTE: we assume black starts
  game->selected_player= &game->b_player;
  
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
//
// NOTE: The texture instantiation is de-ferred to the first call of
// render_piece().
Piece *init_piece(PieceType t, Pos init_pos) {
  assert(t != EMPTY && "Piece shouldn't be EMPTY!");
  
  Piece *p = calloc(1, sizeof(Piece));
  p->pos = init_pos;
  p->type = t;
  p->image = img_p(IMG_Load(type2png(t)));
  
  return p;
}

void destroy_piece(Piece *p) {
  SDL_DestroyTexture(p->texture);
  SDL_FreeSurface(p->image);
  free(p);
}

void update_selected_piece(Game *game, Pos p) {
  // we only update the selected piece if the player is trying to pick
  // his/her own pieces, and not the enemies's.
  Piece *piece = game->board[p.x][p.y];
  
  if (piece) {
    if ((IS_PIECE_BLACK(piece->type) && IS_PLAYER_BLACK(game)) || (IS_PIECE_WHITE(piece->type) && IS_PLAYER_WHITE(game))) {
      game->selected_piece = piece;
    } else {
      game->selected_piece = NULL;
    }
  }

  return;
}

// Computes direction of movement based on starting pos and ending pos.
Dir compute_movement_dir(Pos start_pos, Pos end_pos) {
  int dx = end_pos.x - start_pos.x;
  int dy = end_pos.y - start_pos.y;

  // basic 4-movements
  if (dx == 0 && dy < 0) { return UP;      }
  if (dx == 0 && dy > 0) { return DOWN;    }
  if (dy == 0 && dx < 0) { return LEFT;    }  
  if (dy == 0 && dx > 0) { return RIGHT;   }

  // other 4-diagonal movements
  if (dy < 0  && dx < 0) { return DIAG_LU; }
  if (dy < 0  && dx > 0) { return DIAG_RU; }
  if (dy > 0  && dx < 0) { return DIAG_LD; }
  if (dy > 0  && dx > 0) { return DIAG_RD; }

  fprintf(stderr, "[ERROR] - dx (%d) and dy (%d) are not valid!\n", dx, dy);
  exit(1);
}

int check_move_validity(Game *game, Piece *p, Pos new_pos) {
  // returns 1 if the piece p can move from its current position to
  // new_pos, 0 otherwise.
  Pos old_pos = p->pos;
  int valid = 0;
  Piece *eating_piece = game->board[new_pos.x][new_pos.y];
  Dir movement_dir = compute_movement_dir(p->pos, new_pos);

  int dx = new_pos.x - old_pos.x;
  int dy = new_pos.y - old_pos.y;

  switch(p->type) {

    // at the start the pawn can choose to move two squares below.
    // in general however it can only move one square below.     
  case B_PAWN:
    if (!eating_piece &&
	((old_pos.y == 1 && dy == 2 && dx == 0) || (dy == 1 && dx == 0))) {
      valid = 1;
    }

    else if (eating_piece && dy == 1 && abs(dx) == 1) {
      valid = 1;
    }
    
    break;

  case W_PAWN:
    if (!eating_piece &&
	((old_pos.y == 6 && dy == -2 && dx == 0) || (dy == -1 && dx == 0))) {
      valid = 1;
    }

    else if (eating_piece && dy == -1 && abs(dx) == 1) {
      valid = 1;
    }    
    
    break;

  // -----------
  case B_ROOK:
  case W_ROOK:

    if ((dy && !dx) || (!dy && dx)) {
      return check_obstacles_in_path(game, p->pos, new_pos, movement_dir);
    }
    
    break;

  // -----------
  case B_BISHOP:
  case W_BISHOP:

    if (abs(dx) == abs(dy)) {
      return check_obstacles_in_path(game, p->pos, new_pos, movement_dir);
    }
    
    break;
    
  // -----------
  case B_KNIGHT:
  case W_KNIGHT:
    // TODO: implement this
    break;

  // -----------
  case B_QUEEN:
  case W_QUEEN:
    // TODO: implement this
    break;

  // -----------
  case B_KING:
  case W_KING:
    // TODO: implement this
    break;

  default:
    fprintf(stderr, "[ERROR] - Default clause in check move (%d)!\n", p->type);
    exit(1);
    break;
  }
  
  return valid;
}

// This function should return 1 if the path is 'free of obstacles',
// and 0 otherwise.
//
// To specify a path we need to specify a starting position, an ending
// position, and a direction of movement.  Possible directions are:
int check_obstacles_in_path(Game *game, Pos start_pos, Pos end_pos, Dir dir) {
  switch(dir) {

  case UP:
    // we're moving UP, from higher y-coords to lower y-coords
    for (int y = start_pos.y - 1; y > end_pos.y; y--) {
      if(game->board[start_pos.x][y]) {
	return 0;
      }
    }
    
    break;

  case DOWN:
    // we're moving DOWN, from lower y-coords to higher y-coords
    for (int y = start_pos.y + 1; y < end_pos.y; y++) {
      if(game->board[start_pos.x][y]) {
	return 0;
      }
    }
    break;
    
  case LEFT:
    // we're moving LEFT, from higher x-coords to lower x-coords
    for (int x = start_pos.x - 1; x > end_pos.x; x--) {
      if(game->board[x][start_pos.y]) {
	return 0;
      }
    }    
    
    break;
    
  case RIGHT:
    // we're moving RIGHT, from lower x-coords to higher x-coords
    for (int x = start_pos.x + 1; x < end_pos.x; x++) {
      if(game->board[x][start_pos.y]) {
	return 0;
      }
    }
    
    break;
  
  case DIAG_LU:
    // we're moving on the LEFT-UP DIAGONAL,
    //   from higher x-coords to lower x-coords
    //   from higher y-coords to lower y-coords
    for (int x = start_pos.x - 1, y = start_pos.y - 1; x > end_pos.x && y > end_pos.y; x--, y--) {
      if(game->board[x][y]) {
	return 0;
      }
    }
    
    break;
    
  case DIAG_LD:
    // we're moving on the LEFT-DOWN DIAGONAL,
    //   from higher x-coords to lower x-coords
    //   from lower y-coords to higher y-coords
    for (int x = start_pos.x - 1, y = start_pos.y + 1; x > end_pos.x && y < end_pos.y; x--, y++) {
      if(game->board[x][y]) {
	return 0;
      }
    }
    break;
    
  case DIAG_RU:
    // we're moving on the RIGHT-UP DIAGONAL,
    //   from lower x-coords to higher x-coords
    //   from higher y-coords to lower y-coords
    for (int x = start_pos.x + 1, y = start_pos.y - 1; x < end_pos.x && y > end_pos.y; x++, y--) {
      if(game->board[x][y]) {
	return 0;
      }
    }    
    break;
    
  case DIAG_RD:
    // we're moving on the RIGHT-DOWN DIAGONAL,
    //   from lower x-coords to higher x-coords
    //   from lower y-coords to higher y-coords
    for (int x = start_pos.x + 1, y = start_pos.y + 1; x < end_pos.x && y < end_pos.y; x++, y++) {
      if(game->board[x][y]) {
	return 0;
      }
    }
    break;

  default:
    fprintf(stderr, "[ERROR] - default case shoulnd't be triggered!\n");
    exit(1);
    break;
  }

  return 1;
}

void move_piece(Game *game, Piece *p, Pos new_pos) {
  if(!check_move_validity(&GAME, p, new_pos)) {
    return;
  }
  
  // The move is valid, do it.
  if(game->board[new_pos.x][new_pos.y]) {
    Piece *eaten_piece = game->board[new_pos.x][new_pos.y];
    update_player_score(game->selected_player, eaten_piece->type);
    destroy_piece(eaten_piece);
  }
  
  game->board[p->pos.x][p->pos.y] = NULL;
  game->board[new_pos.x][new_pos.y] = p;
  p->pos = new_pos;

  game->selected_piece= NULL;
  game->selected_player= IS_PLAYER_WHITE(game) ? &game->b_player : &game->w_player;
}

void update_player_score(Player *p, PieceType t) {
  assert(p->score_count < 16 && "score count must be < 16!\n");
  p->score[p->score_count++] = t;
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
    render_pos_highlight(renderer, p->pos);
  }
}

void render_pos_highlight(SDL_Renderer *renderer, Pos pos) {
  sdl2_c(SDL_SetRenderDrawColor(renderer, HEX_COLOR(HIGHLIGHT_COLOR)));

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
  SDL_Window *const window = sdl2_p(SDL_CreateWindow("Description", 0, 0,
						     SCREEN_WIDTH, SCREEN_HEIGHT,
						     SDL_WINDOW_RESIZABLE));
  
  SDL_Renderer *const renderer = sdl2_p(SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED));

  // init image SDL
  IMG_Init(IMG_INIT_PNG);
  init_game(&GAME);

  while(!GAME.quit) {
    SDL_Event event;

    // --------------------
    // start event handling
    while(SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
	GAME.quit = 1;
      }

      if (event.type == SDL_MOUSEBUTTONDOWN) {
	// handle player selection
	Pos new_pos = (Pos){
	  (int) floorf(event.button.x / CELL_WIDTH),
	  floorf(event.button.y / CELL_HEIGHT)
	};

	Piece *p = GAME.board[new_pos.x][new_pos.y];
	if (!GAME.selected_piece || (p && SAME_TYPE(p, GAME.selected_piece))) {
	  update_selected_piece(&GAME, new_pos);
	} else {
	  move_piece(&GAME, GAME.selected_piece, new_pos);
	}
      }
    }
    // --------------------

    // render next frame
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
