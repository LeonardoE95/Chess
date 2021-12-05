#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "./include/game.h"

// ----------------------------------------
// GLOBAL VARIABLES

const PieceType DEFAULT_BOARD[BOARD_HEIGHT][BOARD_WIDTH] = {
    {B_ROOK, B_KNIGHT, B_BISHOP, B_QUEEN, B_KING, B_BISHOP, B_KNIGHT, B_ROOK},
    {B_PAWN, B_PAWN, B_PAWN, B_PAWN, B_PAWN, B_PAWN, B_PAWN, B_PAWN},

    {EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
    {EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
    {EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},
    {EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY},

    {W_PAWN, W_PAWN, W_PAWN, W_PAWN, W_PAWN, W_PAWN, W_PAWN, W_PAWN},
    {W_ROOK, W_KNIGHT, W_BISHOP, W_QUEEN, W_KING, W_BISHOP, W_KNIGHT, W_ROOK},
};

// ----------------------------------------
// FUNCTIONS

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

  game->valid_moves_count = 0;
  
  game->b_player.player_name = B_PLAYER_NAME;
  game->w_player.player_name = W_PLAYER_NAME;
  
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

// ----------

// Used to istantiate a particular chess piece depending on its type.
//
// NOTE: The texture instantiation is de-ferred to the first call of
// render_piece().
Piece *init_piece(PieceType t, Pos init_pos) {
  assert(t != EMPTY && "Piece shouldn't be EMPTY!");
  
  Piece *p = calloc(1, sizeof(Piece));
  p->pos = init_pos;
  p->type = t;
  p->image_path = type2png(t);
  
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
  assert(!out_of_board_pos(p) && "pos should be in board!");

  Piece *piece = game->board[p.x][p.y];
  
  if (piece) {
    if ((IS_PIECE_BLACK(piece->type) && IS_PLAYER_BLACK(game)) || (IS_PIECE_WHITE(piece->type) && IS_PLAYER_WHITE(game))) {
      game->selected_piece = piece;
      update_valid_moves(game);
    } else {
      game->selected_piece = NULL;
    }
  }

  return;
}

// ----------

// Computes direction of movement based on starting pos and ending pos.
Dir compute_movement_dir(Pos start_pos, Pos end_pos) {
  int dx = end_pos.x - start_pos.x;
  int dy = end_pos.y - start_pos.y;

  if (dx == 0 && dy == 0) { return STILL; }
  
  // basic 4-movements
  if (dx == 0 && dy < 0)  { return UP;      }
  if (dx == 0 && dy > 0)  { return DOWN;    }
  if (dy == 0 && dx < 0)  { return LEFT;    }  
  if (dy == 0 && dx > 0)  { return RIGHT;   }

  // other 4-diagonal movements
  if (dy < 0  && dx < 0)  { return DIAG_LU; }
  if (dy < 0  && dx > 0)  { return DIAG_RU; }
  if (dy > 0  && dx < 0)  { return DIAG_LD; }
  if (dy > 0  && dx > 0)  { return DIAG_RD; }

  fprintf(stderr, "[ERROR] - dx (%d) and dy (%d) are not valid!\n", dx, dy);
  exit(1);
}

int out_of_board_pos(Pos pos) {
  // Returns 1 if `pos` is out of the board.

  if (pos.x < 0 || pos.x >= 8) {
    return 1;
  }

  if (pos.y < 0 || pos.y >= 8) {
    return 1;
  }
  
  return 0;
}

int check_move_validity(Game *game, Piece *p, Pos new_pos) {
  // returns 1 if the piece p can move from its current position to
  // new_pos, 0 otherwise.
  Pos old_pos = p->pos;
  Piece *eating_piece = game->board[new_pos.x][new_pos.y];
  Dir movement_dir = compute_movement_dir(p->pos, new_pos);

  if (movement_dir == STILL || out_of_board_pos(new_pos)) {
    return 0; // edge-case cases
  }
  
  int dx = new_pos.x - old_pos.x;
  int dy = new_pos.y - old_pos.y;

  switch(p->type) {

    // at the start the pawn can choose to move two squares below.
    // in general however it can only move one square below.     
  case B_PAWN:
    if (!eating_piece &&
	((old_pos.y == 1 && dy == 2 && dx == 0) || (dy == 1 && dx == 0))) {
      return 1;
    }

    else if (eating_piece && dy == 1 && abs(dx) == 1) {
      return 1;
    }
    
    break;

  case W_PAWN:
    if (!eating_piece &&
	((old_pos.y == 6 && dy == -2 && dx == 0) || (dy == -1 && dx == 0))) {
      return 1;
    }

    else if (eating_piece && dy == -1 && abs(dx) == 1) {
      return 1;
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
    // NOTE: here we don't have to check for obstacles.
    if ((dy == -2 && (dx == -1 || dx == 1)) || (dy == 2 && (dx == -1 || dx == 1))  ||
	(dx == -2 && (dy == -1 || dy == 1)) || (dx == 2 && (dy == -1 || dy == 1))) {
      return 1;
    }
    
    break;

  // -----------
  case B_QUEEN:
  case W_QUEEN:
    
    if ((abs(dx) == abs(dy)) || (dy && !dx) || (!dy && dx)) {
      return check_obstacles_in_path(game, p->pos, new_pos, movement_dir);
    }

    break;

  // -----------
  case B_KING:
  case W_KING:

    if ((dy == 0 || dy == 1 || dy == -1) && (dx == 0 || dx == 1 || dx == -1)) {
      return 1;
    }
    
    break;

  default:
    fprintf(stderr, "[ERROR] - Default clause in check move (%d)!\n", p->type);
    exit(1);
    break;
  }

  return 0;
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

int move_piece(Game *game, Piece *p, Pos new_pos) {
  int finished = 0;
  Piece *eaten_piece = game->board[new_pos.x][new_pos.y];
  
  if(!check_move_validity(game, p, new_pos)) {
    return finished;
  }
  
  // The move is valid, do it.
  if(eaten_piece) {
    update_player_score(game->selected_player, eaten_piece->type);

    // check if game is over.
    finished = eaten_piece->type == B_KING || eaten_piece->type == W_KING;
    
    destroy_piece(eaten_piece);
  }
  
  game->board[p->pos.x][p->pos.y] = NULL;
  game->board[new_pos.x][new_pos.y] = p;
  p->pos = new_pos;

  game->selected_piece = NULL;

  if (!finished) {
    // only change player if game is over
    game->selected_player = IS_PLAYER_WHITE(game) ? &game->b_player : &game->w_player;
  }

  // reset valid positions
  game->valid_moves_count = 0;

  return finished;
}

// ----------

void update_player_score(Player *p, PieceType t) {
  assert(p->score_count < 16 && "score count must be < 16!\n");
  p->score[p->score_count++] = t;
}

// ----------

void add_valid_move(Game *game, Pos new_pos) {
  if (game->valid_moves_count + 1 >= MAX_VALID_MOVES) {
    fprintf(stderr, "[ERROR] - Array valid_moves completely filled!");
    exit(1);
  }

  game->valid_moves[game->valid_moves_count++] = new_pos;

  return;
}

// Determines which moves are valid out of all possible moves
// depending on the current selected piece.
void update_valid_moves(Game *game) {
  game->valid_moves_count = 0;
  
  if (!game->selected_piece) {
    // No piece is selected, therefore no moves are valid.
    return; 
  }

  // iterate over all possible position and check if piece can be
  // moved there.
  for (int x = 0; x < BOARD_WIDTH; x++) {
    for (int y = 0; y < BOARD_HEIGHT; y++) {
      Pos p = (Pos) {.x = x, .y = y};
      Piece *eating_piece = game->board[p.x][p.y];

      // TODO: instead of checking here if we're trying to move on the
      // player's own pieces, we should instead do that within the
      // check_move_validity().
      if (check_move_validity(game, game->selected_piece, p) &&
	  (!eating_piece || (eating_piece && !SAME_PLAYER(eating_piece, game->selected_piece)))) {
	add_valid_move(game, p);
      }
    }
  }

  return;
}
