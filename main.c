// As Kaze Emanuar once said, "Make as many assumptions as possible in your code"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned long long GLOBAL_TIMER = 0; // 256 is considered "One second"

#if defined __unix__ || defined __APPLE__ && defined __MACH__
#define USING_TERMIOS
#include <termios.h>
#else
#define __USING_CONIO
#include <conio.h>
#define __USING_WINDOWS
#include <windows.h>
#endif

#include "gamestate.h"
#include "keyhandler.h"
#include "object.h"
#include "screen.h"
#include "generation.h"
#include "acts.h"

void runIntro(ENTITY *game) {
    B_BUFFER *BUFFER;
    GetDataFlag(game, FLAG_APPEARANCE, (void**)&BUFFER);
    b_writeTo(BUFFER, 0, 0, "Welcome to NOTE.");
    b_writeTo(BUFFER, 4, 0, "What is your name?");
    b_draw(BUFFER);
    s_putCursor(4, 20);
    char *PlayerName;
    getStringInput((char**)&PlayerName);
    SetDataFlag(game, FLAG_NAME, &PlayerName);
    int *place = malloc(sizeof(int));
    *place = PLAYING;
    SetDataFlag(game, FLAG_PLACE, (void*)place);
}

void runGame(ENTITY *game) {
    B_BUFFER *buffer;
    int leadAct = 0x0;
    int tailAct = 0x0;
    char *commAct;
    int *CSTATE = 0;
    while (!CSTATE || *CSTATE == PLAYING) {
        GetDataFlag(game, FLAG_PLACE, (void**)&CSTATE);
        if (*CSTATE != PLAYING)
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
        GetDataFlag(game, FLAG_APPEARANCE, (void**)&buffer);
        if (gotMap) {
            b_writeMapToBuffer(buffer, map);
        }
        b_draw(buffer);
        s_putCursor(S_ROW, S_COL - 5);
        leadAct = getNextInput();
        char requiresOtherAct = 0;
        switch (leadAct) {
			case FAIL:
				break;
			case STR_COMM: {
				s_putCursor(0, 0);
				getStringInput((char**)&commAct);
				if(stringEqCaseless(commAct, "quit") || stringEqCaseless(commAct, "exit")) {
					int *place = malloc(sizeof(int));
					*place = QUIT;
					SetDataFlag(game, FLAG_PLACE, &place);
					return;
				}
			}
			break;
			case UP:
			case DOWN:
			case LEFT:
			case RIGHT:
			case UPLEFT:
			case UPRIGHT:
			case DOWNLEFT:
			case DOWNRIGHT: {
				TryMove(player, leadAct);
				break;
			}
			case UP_KEEP:
			case DOWN_KEEP:
			case LEFT_KEEP:
			case RIGHT_KEEP:
			case UPLEFT_KEEP:
			case UPRIGHT_KEEP:
			case DOWNLEFT_KEEP:
			case DOWNRIGHT_KEEP: {
				while(TryMove(player, leadAct)) {}
				break;
			}
			case BUFFER_REDRAW: {
				b_flush(buffer);
				break;
			}
			default: {
				requiresOtherAct = 1;
				break;
			}
		}
		if(requiresOtherAct) {
			tailAct = getNextInput();
		}
	}
}

int main(void) {
	s_clearScreen();
	ENTITY *game;
	generateGame(&game);
	runIntro(game);
	runGame(game);
	return 0;
}