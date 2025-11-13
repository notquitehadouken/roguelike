// As Kaze Emanuar once said, "Make as many assumptions as possible in your code"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined __unix__ || defined __APPLE__ && defined __MACH__
#define USING_TERMIOS
#include <termios.h>
#else
#define __USING_CONIO
#include <conio.h>
#define __USING_WINDOWS
#include <windows.h>
#endif

#include "global.h"
#include "gamestate.h"
#include "keyhandler.h"
#include "object.h"
#include "screen.h"
#include "generation.h"
#include "acts.h"

void runIntro(ENTITY* game)
{
  B_BUFFER* BUFFER;
  GetDataFlag(game, FLAG_APPEARANCE, (void**)&BUFFER);
  b_writeTo(BUFFER, 0, 0, "Welcome to NOTE.");
  b_writeTo(BUFFER, 4, 0, "What is your name?");
  b_draw(BUFFER);
  char *PlayerName;
  b_stringInputAt(4, 20, BUFFER, &PlayerName);
  b_draw(BUFFER);
  SetDataFlag(game, FLAG_NAME, &PlayerName);
  int *place = malloc(sizeof(int));
  *place = PLAYING;
  SetDataFlag(game, FLAG_PLACE, place);
}

int actNext = 0;

void runGame(ENTITY* game)
{
  B_BUFFER* buffer;
  int leadAct = 0x0;
  int tailAct = 0x0;
  char* commAct;
  int* CSTATE = nullptr;
  while (!CSTATE || *CSTATE == PLAYING)
  {
    GetDataFlag(game, FLAG_PLACE, (void**)&CSTATE);
    if (*CSTATE != PLAYING)
      break;
    leadAct = 0;
    tailAct = 0;
    ENTITY** mapRef;
    char gotMap = 0;
    ENTITY* player;
    GetDataFlag(game, FLAG_PLAYER, (void**)&player);
    while (!gotMap)
    {
      GetDataFlag(player, FLAG_CONTAINEDBY, (void**)&mapRef);
      if (!mapRef)
        break;
      GetBoolFlag(*mapRef, BFLAG_ISMAP, &gotMap);
    }
    const ENTITY* map = *mapRef;
    GetDataFlag(game, FLAG_APPEARANCE, (void**)&buffer);
    if (gotMap)
    {
      b_writeMapToBuffer(buffer, map, player);
    }
    b_writeHudToBuffer(buffer, player);
    b_draw(buffer);
    s_putCursor(S_ROW, S_COL - 5);
    leadAct = actNext;
    if (!leadAct)
      leadAct = getNextInput();
    switch (leadAct)
    {
    case FAIL:
      break;
    case STR_COMM:
      {
        b_stringInputAt(0, 0, buffer, (char**)&commAct);
        if (stringEqCaseless(commAct, "quit") || stringEqCaseless(commAct, "exit"))
        {
          int* place = malloc(sizeof(int));
          *place = QUIT;
          SetDataFlag(game, FLAG_PLACE, &place);
          return;
        }
        break;
      }
    case UP:
    case DOWN:
    case LEFT:
    case RIGHT:
    case UPLEFT:
    case UPRIGHT:
    case DOWNLEFT:
    case DOWNRIGHT:
      {
        GTimeAdvance(TryMove(player, leadAct), player);
        break;
      }
    case UP_KEEP:
    case DOWN_KEEP:
    case LEFT_KEEP:
    case RIGHT_KEEP:
    case UPLEFT_KEEP:
    case UPRIGHT_KEEP:
    case DOWNLEFT_KEEP:
    case DOWNRIGHT_KEEP:
      {
        const int moveTime = TryMove(player, leadAct);
        if (moveTime >= 0)
        {
          GTimeAdvance(moveTime, player);
          actNext = leadAct;
        }
        else
        {
          actNext = 0;
        }
        break;
      }
    case LONG_WAIT:
      {
        GTimeAdvance(TIMEOF_LONGWAIT, player);
        break;
      }
    case SHORT_WAIT:
      {
        GTimeAdvance(TIMEOF_SHORTWAIT, player);
        break;
      }
    case BUFFER_REDRAW:
      {
        b_flush(buffer);
        break;
      }
    default:
      {
        break;
      }
    }

    for (;;)
    {
      int anyProcessed = 0;
      for (int uid = 1;; uid++)
      {
        ENTITY* ent = EntityLookup(uid);
        if (!ent)
          break;

        ENTITY_CONTROLLER* Controller;
        GetDataFlag(ent, FLAG_CONTROLS, (void**)&Controller);
        if (!Controller)
          continue;

        if (Controller->nextAct > GLOBAL_TIMER)
          continue;

        anyProcessed |= ControllerProcess(Controller, game);
      }
      if (!anyProcessed)
        break;
    }
  }
}
int main(void)
{
  s_clearScreen();
  ENTITY* game;
  generateGame(&game);
  runIntro(game);
  runGame(game);
  return 0;
}
