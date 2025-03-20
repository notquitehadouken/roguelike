#pragma once
#define S_COL 60
#define S_ROW 40
#define S_LENGTH S_ROW*S_COL
#define MAP_WIDTH S_COL
#define MAP_HEIGHT (S_ROW-5)
#define MAP_LENGTH MAP_WIDTH*MAP_HEIGHT

enum ENTITY_STATE{
    CREATION = 0, // the game has just been started
    PLAYING, // the game is being played
    QUIT, // time to leave
};
