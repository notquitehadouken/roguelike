#pragma once
#define S_COL 80
#define S_ROW 60
#define S_LENGTH S_ROW*S_COL
#define MAP_WIDTH S_COL
#define MAP_HEIGHT (S_ROW-5)
#define MAP_DEPTH 0x100 // this can't really be higher than 0x100...
#define MAP_CELLCOUNT MAP_WIDTH*MAP_HEIGHT
#define MAP_IND(X, Y) (Y * MAP_WIDTH + X)

enum ENTITY_STATE
{
  CREATION = 0, // the game has just been started
  PLAYING, // the game is being played
  QUIT, // time to leave
};
