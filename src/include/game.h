#ifndef GAME_H_
#define GAME_H_

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#define SCREEN_WIDTH  600
#define SCREEN_HEIGHT 600

#define BOARD_WIDTH 8
#define BOARD_HEIGHT 8

#define CELL_WIDTH ((SCREEN_WIDTH / BOARD_WIDTH))
#define CELL_HEIGHT ((SCREEN_HEIGHT / BOARD_HEIGHT))

// represents maximum amount of different moves a single piece can do
// at any given time.
#define MAX_VALID_MOVES (BOARD_WIDTH * 4)

#define B_PLAYER_NAME "BLACK"
#define W_PLAYER_NAME "WHITE"

// ----------------------------------------
// DATA STRUCTURES

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

  STILL,
} Dir;

typedef struct {
  int x;
  int y;
} Pos;

typedef struct {
  PieceType type;
  Pos pos;
  const char *image_path;

  SDL_Surface *image;
  SDL_Texture *texture;
} Piece;

typedef struct {
  PieceType score[16];
  int score_count;
  char *player_name;
} Player;

typedef struct {
  Piece *board[BOARD_WIDTH][BOARD_HEIGHT];
  
  // NOTE: at most a piece can move in <= 8 * 4 = 32 different positions
  Pos valid_moves[MAX_VALID_MOVES];
  int valid_moves_count;

  Player b_player;
  Player w_player;

  Piece *selected_piece;
  Player *selected_player;
  
  int quit;
} Game;

// ----------------------------------------
// DECLARATIONS

const char *type2png(PieceType t);

void init_game(Game *game);
void destroy_game(Game *game);

Piece *init_piece(PieceType t, Pos init_pos);
void update_selected_piece(Game *game, Pos p);
void destroy_piece(Piece *p);

int check_move_validity(Game *game, Piece *p, Pos new_pos);
int move_piece(Game *game, Piece *p, Pos new_pos);
Dir compute_movement_dir(Pos start_pos, Pos end_pos);
int out_of_board_pos(Pos pos);
int check_obstacles_in_path(Game *game, Pos start_pos, Pos end_pos, Dir dir);

void update_player_score(Player *p, PieceType t);

void update_valid_moves(Game *game);
void add_valid_move(Game *game, Pos new_pos);

// ----------------------------------------
// UTILS MACRO

#define IS_PIECE_BLACK(x) (x >= 0 && x <= 5)
#define IS_PIECE_WHITE(x) (x >= 6 && x <= 11)

#define IS_PLAYER_BLACK(g) (g->selected_player == &g->b_player)
#define IS_PLAYER_WHITE(g) (g->selected_player == &g->w_player)

#define SAME_PLAYER(p1, p2) ((IS_PIECE_BLACK(p1->type) && IS_PIECE_BLACK(p2->type)) || (IS_PIECE_WHITE(p1->type) && IS_PIECE_WHITE(p2->type)))

#endif // GAME_H_
