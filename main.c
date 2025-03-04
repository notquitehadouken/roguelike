// As Kaze Emanuar once said, "Make as many assumptions as possible in your code"
#define DEBUG
#define VIOLENT_DEBUG

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

#include "keyhandler.h"
#include "screen.h"
#include "object.h"
#include "gamestate.h"
#include "generation.h"

void runIntro(ENTITY *game) {
	B_BUFFER *BUFFER;
	GetDataFlag(game, FLAG_APPEARANCE, (void**)&BUFFER);
	b_writeTo(BUFFER, 0, 0, "Welcome to NOTE.");
	b_writeTo(BUFFER, 4, 0, "What is your name?");
	b_draw(BUFFER);
	s_putCursor(4, 20);
	char PlayerName[MAX_STR_LEN];
	getStringInput(&PlayerName);
	SetDataFlag(game, FLAG_NAME, &PlayerName);
	SetDataFlag(game, FLAG_PLACE, (void*)PLAYING);
}

void runGame(ENTITY *game) {
	B_BUFFER *buffer;
	GetDataFlag(game, FLAG_APPEARANCE, (void**)&buffer);
	b_draw(buffer);
	char leadAct = 0x0;
	char tailAct = 0x0;
	char commAct[MAX_STR_LEN];
	int CSTATE = PLAYING;
	while (CSTATE == PLAYING) {
		GetDataFlag(game, FLAG_PLACE, (void**)&CSTATE);
		if (CSTATE != PLAYING)
			break;
		leadAct = 0;
		tailAct = 0;
		ENTITY *map;
		char gotMap = 0;
		ENTITY *player;
		GetDataFlag(game, FLAG_PLAYER, (void**)&player);
		while (!gotMap) {
			GetDataFlag(player, FLAG_CONTAINEDBY, (void**)&map);
			if (!map)
				break;
			GetBoolFlag(map, BFLAG_ISMAP, &gotMap);
		}
		if (gotMap) {
			b_writeMapToBuffer(buffer, map);
			b_draw(buffer);
		}
		s_putCursor(S_ROW, S_COL - 5);
		leadAct = getNextInput();
		if(leadAct == FAIL)
			continue;
		if(leadAct == STR_COMM) {
			s_putCursor(0, 0);
			getStringInput((char**)&commAct);
			if(stringEqCaseless(commAct, "quit") || stringEqCaseless(commAct, "exit")) {
				SetDataFlag(game, FLAG_PLACE, (void**)QUIT);
				return;
			}
			continue;
		}
		if(!qualifiesSolo(leadAct) ) {
			tailAct = getNextInput();
		}
	}
}

int main(void) {
	setvbuf(stdin, NULL, _IONBF, 0);
	ENTITY *game;
	generateGame(&game);
	runIntro(game);
	runGame(game);
	return 0;
}