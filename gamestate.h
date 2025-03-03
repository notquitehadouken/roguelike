#pragma once
#define MAP_WIDTH 80
#define MAP_HEIGHT 45
#define MAP_LENGTH MAP_WIDTH*MAP_HEIGHT

enum __STATE{
	CREATION = 0, // the game has just been started
	PLAYING, // the game is being played
	QUIT, // time to leave
};
