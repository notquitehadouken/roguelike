// As Kaze Emanuar once said, "Make as many assumptions as possible in your code"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <threads.h>

#if defined DEBUG_MESSAGE_SILENCE\
  || defined DEBUG_MESSAGE_SHOW
#define DEBUG_OPTIONS_ENABLED
#else
#undef DEBUG_OPTIONS_ENABLED
#endif


#if defined __unix__ || defined __APPLE__ && defined __MACH__
#define USING_TERMIOS
#else
#define USING_CONIO
#define USING_WINDOWS
#endif

#define attrpure __attribute_pure__
#define attrconst __attribute_const__
#define attrmalloc __attribute_malloc__

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
  int titleTextDrawRow = 4;
  int titleTextDrawCol = 3;
  static const char TheFuckingTitleText[] =
"                      *                                                **\n"
"      *              **                           ******        *********\n"
"      **             **                      ***********   *************\n"
"     ***             **               **********     ** ***********  **\n"
"    *****           **               *****    **    ** *********     **\n"
"   *  ***           **              ****      **       ***           *\n"
"      ***           **    *********   ***    ***       **\n"
"      ****         **   ****    *****        ***      **\n"
"      ****         **  **          ***       ***      **\n"
"      ****        **  **            ***      ***      **\n"
"      ****        ** ***             **      ***     ***\n"
"      *  *        ** ***              **     **      ***        *****\n"
"      *  **      *** **               ***    **      **      *******\n"
"     **  **      ** ***                **   ***     ***    ******\n"
"     **   **     ** **                 **   **      *********\n"
"    ***   **     *  **                 **   **      *******\n"
"    **    **     *  **                ***  ***      ****\n"
"    **    ***    *  **                ***  ***      **\n"
"    **     **   ** **                 ***  **       **\n"
"    **     **   ** **                 **   **       **               *\n"
"   ***     ***  ** **                 **   **      ***               ***\n"
"   ***      ** *** ***               ***   **      ***                ****\n"
"   ***      *****   **               **   ***      **             *******\n"
"   **        ****   **              ***   ***      **            ******\n"
"   **         **     **             ***   **      ***          *****\n"
"  ***         **     **            ***    ***     ***       *****\n"
"  ***         **     ***          ***    *****    ***    ******\n"
"  **          *       **         **     ********  *** *******\n"
"  **                   ***    ****     ***        *********\n"
" ***                     ******       **          ****\n"
" **                                               *\n"
"**\n"
"*\n";
  int i;
  for (i = 0; TheFuckingTitleText[i]; i++)
  {
    if (TheFuckingTitleText[i] == '\n')
    {
      titleTextDrawCol = 3  ;
      titleTextDrawRow += 1;
      continue;
    }
    if (TheFuckingTitleText[i] != ' ')
    {
      B_PIXEL PxD = {'*', 235, 0, 0};
      b_setPixel(BUFFER, titleTextDrawRow + 1, titleTextDrawCol - 1, &PxD);
      PxD.color = 240;
      b_setPixel(BUFFER, titleTextDrawRow, titleTextDrawCol, &PxD);
      PxD.color = 255;
      b_setPixel(BUFFER, titleTextDrawRow - 1, titleTextDrawCol + 1, &PxD);
    }
    titleTextDrawCol += 1;
  }
  for (i = 0; i < S_ROW; i++)
  {
    char *left = (i % 2) ? "\\" : "/";
    char *right = (i % 2) ? "/" : "\\";
    b_writeTo(BUFFER, i, 0, left);
    b_writeTo(BUFFER, i, S_COL - 1, right);
  }
  b_writeTo(BUFFER, S_ROW - 2, 1, "/\n/");
  b_writeTo(BUFFER, S_ROW - 2, S_COL - 2, "\\\n\\");
  b_writeTo(BUFFER, 0, 1, "\\\n\\");
  b_writeTo(BUFFER, 0, S_COL - 2, "/\n/");
  for (i = 2; i < S_COL - 2; i++)
  {
    b_writeTo(BUFFER, S_ROW - 3, i, "_\n_");
    b_writeTo(BUFFER, 0, i, "_\n_");
  }
  b_draw(BUFFER);
  char* PlayerName;
  b_stringInputAt(0, 0, BUFFER, &PlayerName);
  b_draw(BUFFER);
  entSetName(game, PlayerName);
  SetDataFlag(game, FLAG_PLACE, intap(PLAYING));
}

void runGame(ENTITY* game)
{
  B_BUFFER* buffer;
  char* commAct;
  int* CSTATE = 0;
int leadAct = 0x0;
  int actNext = 0x0;
  while (!CSTATE || *CSTATE == PLAYING)
  {
    GetDataFlag(game, FLAG_PLACE, (void**)&CSTATE);
    if (*CSTATE != PLAYING)
      break;
    int i;
    for (i = 0; i < MAP_CELLCOUNT; i++)
    {
      cacheShrink(_CACHE_REF(POSITION_CACHE + i));
    }

    ENTITY* player;
    GetDataFlag(game, FLAG_PLAYER, (void**)&player);
    _CACHE_OF(ENTITY)* mapContainer;
    ENTITY* map = mapOf(player);
    GetDataFlag(map, FLAG_CONTAINER, (void**)&mapContainer);
    cacheShrink(_CACHE_REF(mapContainer));
    GetDataFlag(game, FLAG_APPEARANCE, (void**)&buffer);
    if (map)
    {
      b_writeMapToBuffer(buffer, map, player);
    }
    b_writeHudToBuffer(buffer, player);
    b_draw(buffer);
    s_putCursor(S_ROW, S_COL - 5);
    leadAct = actNext;
    if (!leadAct)
      leadAct = getNextInput();
    actNext = 0;

    int PlayerX, PlayerY, PlayerZ;
    int *PlayerPos;
    GetDataFlag(player, FLAG_POS, (void**)&PlayerPos);
    ConvertToZXY(*PlayerPos, &PlayerZ, &PlayerX, &PlayerY);

    switch (leadAct)
    {
    case 0:
    case FAIL:
      break;
    case STR_COMM:
      {
        b_stringInputAt(0, 0, buffer, &commAct);
        if (stringEqCaseless(commAct, "quit") || stringEqCaseless(commAct, "exit"))
        {
          SetDataFlag(game, FLAG_PLACE, intap(QUIT));
          return;
        }
        break;
      }
    case OPEN_HELP:
      {
        b_factory(buffer);
        char TheHelpText[S_COL + 1][S_ROW] = {};
        sprintf(TheHelpText[0], " %c  - Open this dialog", OPEN_HELP);
        sprintf(TheHelpText[2], "%c%c%c - Move. ", UPLEFT, UP, UPRIGHT);
        if (JUMP == ' ')
        {
          sprintf(TheHelpText[2] + strlen(TheHelpText[2]), "Space to jump.");
        }
        else
        {
          sprintf(TheHelpText[2] + strlen(TheHelpText[2]), "%c to jump.", JUMP);
        }
        sprintf(TheHelpText[3], "%c %c", LEFT, RIGHT);
        sprintf(TheHelpText[4], "%c%c%c", DOWNLEFT, DOWN, DOWNRIGHT);
#ifndef NUMPAD_USE
        sprintf(TheHelpText[4] + 3, "   Hold shift to continue moving.");
#endif
        sprintf(TheHelpText[6], "%c %c - Move down and up", HEIGHTDOWN, HEIGHTUP);
        sprintf(TheHelpText[8], "%c,%c - Wait (short and long)", SHORT_WAIT, LONG_WAIT);
        sprintf(TheHelpText[10], "%c %c - Look down and up", KEY_VIEWHEIGHT_DOWN, KEY_VIEWHEIGHT_UP);
        sprintf(TheHelpText[11], "%c %c - Custom view height mode", KEY_VIEWHEIGHT_C_LOWER, KEY_VIEWHEIGHT_C_RAISE);
        sprintf(TheHelpText[12], " %c  - Toggle color scaling mode", KEY_MAPCOLORSCALINGTOGGLE);
        sprintf(TheHelpText[14], " %c  - Open inventory", INVENTORY);

        for (i = 0; i < S_ROW; i++)
        {
          b_writeTo(buffer, i, 0, TheHelpText[i]);
        }
        b_flush(buffer);
        getck(&l_I);
        break;
      }
    case EXAMINE:
      {
        int X = PlayerX, Y = PlayerY, Z = PlayerZ;
        PositionSelect(buffer, map, player, &X, &Y, &Z);
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
        const int moveTime = TryMove(player, leadAct);
        if (moveTime >= 0)
        {
          GTimeAdvance(moveTime);
        }
        break;
      }
    case HEIGHTUP:
    case HEIGHTDOWN:
      {
        int dir = 0;
        if (leadAct == HEIGHTUP)
          dir = 1;
        else
          dir = -1;
        const int moveTime = TryMoveVertical(player, dir);
        if (moveTime >= 0)
        {
          GTimeAdvance(moveTime);
        }
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
          GTimeAdvance(moveTime);
          actNext = leadAct;
        }
        break;
      }
    case HEIGHTUP_KEEP:
    case HEIGHTDOWN_KEEP:
      {
        int dir = 0;
        if (leadAct == HEIGHTUP_KEEP)
          dir = 1;
        else
          dir = -1;
        const int moveTime = TryMoveVertical(player, dir);
        if (moveTime >= 0)
        {
          GTimeAdvance(moveTime);
          actNext = leadAct;
        }
        break;
      }
    case JUMP:
      {
        TryJump(player);
        break;
      }
    case KEY_MAPCOLORSCALINGTOGGLE:
      {
        if (mapColorScaling == COLORSCALING_STANDARD)
          mapColorScaling = COLORSCALING_LOW;
        else
          mapColorScaling = COLORSCALING_STANDARD;
        b_flush(buffer);
        break;
      }
    case KEY_VIEWHEIGHT_C_RAISE:
      {
        if (mapViewHeight != VIEWHEIGHT_CUSTOM)
          mapCustomVH = GetEntZ(player);
        else
          mapCustomVH = min(mapCustomVH + 1, 255);
        mapViewHeight = VIEWHEIGHT_CUSTOM;
        break;
      }
    case KEY_VIEWHEIGHT_C_LOWER:
      {
        if (mapViewHeight != VIEWHEIGHT_CUSTOM)
          mapCustomVH = GetEntZ(player);
        else
          mapCustomVH = max(mapCustomVH - 1, 0);
        mapViewHeight = VIEWHEIGHT_CUSTOM;
        break;
      }
    case KEY_VIEWHEIGHT_UP:
      {
        if (mapViewHeight == VIEWHEIGHT_CUSTOM)
          mapViewHeight = VIEWHEIGHT_LEVEL;
        else if (mapViewHeight < VIEWHEIGHT_HIGH)
          mapViewHeight++;
        break;
      }
    case KEY_VIEWHEIGHT_DOWN:
      {
        if (mapViewHeight == VIEWHEIGHT_CUSTOM)
          mapViewHeight = VIEWHEIGHT_LEVEL;
        else if (mapViewHeight > VIEWHEIGHT_LOW)
          mapViewHeight--;
        break;
      }
    case LONG_WAIT:
      {
        GTimeAdvance(TIMEOF_LONGWAIT);
        break;
      }
    case SHORT_WAIT:
      {
        GTimeAdvance(TIMEOF_SHORTWAIT);
        break;
      }
    case BUFFER_REDRAW:
      {
        b_flush(buffer);
        break;
      }
    case DEBUG_ENT:
    case DEBUG_ENTS_ON_POS:
      {
        // Haha! No.
        break;
      }
    case I_CAST_EXPLODE_BALLS:
      {
        int X = PlayerX, Y = PlayerY, Z = PlayerZ;
        PositionSelect(buffer, map, player, &X, &Y, &Z);
        Explode(map, 4, 20, X, Y, Z);
        break;
      }
    case I_CAST_GUN:
      {
        int X = PlayerX, Y = PlayerY, Z = PlayerZ;
        PositionSelect(buffer, map, player, &X, &Y, &Z);
        ShootBullet(player, 1, X - PlayerX,Y - PlayerY,Z - PlayerZ);
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
      int uid;
      for (uid = 1;; uid++)
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
#if defined DEBUG_OPTIONS_ENABLED && !defined DEBUG_MESSAGE_SILENCE
  puts("Some debug options are enabled:");
#ifdef DEBUG_MESSAGE_SHOW
  puts("DEBUG_MESSAGE_SHOW");
#endif
  puts("Press enter to continue, press escape to exit.");
  while (getck(&l_I))
  {
    if (l_I == '\033')
      exit(0);
    if (l_I == '\n')
      break;
  }
#endif
  PopulateCacheArrays();
  s_clearScreen();
  ENTITY* game;
  generateGame(&game);
  runIntro(game);
  runGame(game);
  s_clearScreen();
  s_putCursor(0, 0);
  getck(&l_I);
  return 0;
}
