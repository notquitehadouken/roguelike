#include <stdio.h>
#include <stdbool.h>
#include "input.h"
#include "screen.h"
#include "object.h"
#include "gamestate.h"

void loop(game_state *game) {
	game->state = OVER; // Give up.
	display(game->screen);
}

int main(void) {
	game_state game;
	game.state = MENU;
	game.screen = createScreen();
	while (game.state != OVER)
		loop(&game);
	return 0;
}
