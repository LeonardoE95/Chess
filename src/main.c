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

#include "./include/game.h"
#include "./include/render.h"

// ----------------------------------------
// GLOBALS

Game GAME = {0};

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
	Pos new_pos = (Pos) {
	  (int) floorf(event.button.x / CELL_WIDTH),
	  (int) floorf(event.button.y / CELL_HEIGHT) };

	// skip if out of board
	if (out_of_board_pos(new_pos)) {
	  continue;
	}

	Piece *p = GAME.board[new_pos.x][new_pos.y];
	
	if (!GAME.selected_piece || (p && SAME_PLAYER(p, GAME.selected_piece))) {
	  // player has picked up a piece
	  update_selected_piece(&GAME, new_pos);

	} else {
	  // player has moved a piece
	  int finished = move_piece(&GAME, GAME.selected_piece, new_pos);

	  if (finished) {
	    printf("Game is over: Player %s won!\n", GAME.selected_player->player_name);
	    printf("Resetting ...\n\n");
	    destroy_game(&GAME);
	    init_game(&GAME);
	  }
	}
      }
    }

    // render next frame
    render_game(renderer, &GAME);
  }

  destroy_game(&GAME);
  
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  
  IMG_Quit();
  SDL_Quit();
}
