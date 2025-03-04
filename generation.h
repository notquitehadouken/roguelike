#pragma once
#define CONTAINERCAPACITY 4096
#define GAMETOTALMAPS 256
#include "object.h"

extern void addEntToContainer(ENTITY *container, ENTITY *ent) {
	ENTITY **ELIST; // (-1) [cursed]
	GetDataFlag(container, FLAG_CONTAINER, (void**)&ELIST);
	for (int i = 0; i < CONTAINERCAPACITY /* If it hits CONTAINERCAPACITY entities,
		you are fucked.*/; i++) {
		if (!ELIST[i]) {
			SetDataFlag(ent, FLAG_CONTAINEDBY, container);
			ELIST[i] = ent;
			return;
		}
	}
}

extern void removeEntFromContainer(ENTITY *container, ENTITY *ent) {
	ENTITY **ELIST; // (-1) [cursed]
	GetDataFlag(container, FLAG_CONTAINER, (void**)&ELIST);
	for (int i = 0; i < CONTAINERCAPACITY; i++) {
		if (ELIST[i] == ent) {
			ClearDataFlag(ent, FLAG_CONTAINEDBY);
			ELIST[i] = 0;
			return;
		}
	}
}

extern void addContainer(ENTITY *E) {
	if (HasDataFlag(E, FLAG_CONTAINER))
		return;
	void **LIST = malloc(CONTAINERCAPACITY * sizeof(int*));
	for (int i = 0; i < CONTAINERCAPACITY; i++)
		LIST[i] = 0;
	SetDataFlag(E, FLAG_CONTAINER, LIST);
}

extern void removeContainer(ENTITY *E) {
	ENTITY **ELIST;
	GetDataFlag(E, FLAG_CONTAINER, (void**)&ELIST);
	if (!ELIST)
		return;
	ClearDataFlag(E, FLAG_CONTAINER); // Time to rescue all the entities from the void.
	ENTITY *CONTAINING_ENT;
	GetDataFlag(E, FLAG_CONTAINEDBY, (void**)&CONTAINING_ENT);
	for (int i = 0; i < CONTAINERCAPACITY; i ++) {
		ENTITY *E2 = ELIST[i];
		if (!E2)
			break;
		ClearDataFlag(E2, FLAG_CONTAINEDBY);
		if (CONTAINING_ENT)
			addEntToContainer(CONTAINING_ENT, E2);
	}
}

extern void generateGame(ENTITY **out) {
	ENTITY *game;
	CreateEntity(&game);
	ENTITY *map;
	CreateEntity(&map);
	SetBoolFlag(map, BFLAG_ISMAP);
	addContainer(map);
	ENTITY **maps = malloc(GAMETOTALMAPS * sizeof(map));
	maps[0] = map;
	SetDataFlag(game, FLAG_MAPS, maps);
	for (int x = 0; x < MAP_WIDTH; x++)
	for (int y = 0; y < MAP_HEIGHT; y++) {
		ENTITY *floorTile;
		CreateEntity(&floorTile);
		B_PIXEL *floorPixel = malloc(sizeof(floorPixel));
		floorPixel->text = '.';
		floorPixel->color = 30;
		SetDataFlag(floorTile, FLAG_APPEARANCE, floorPixel);
		unsigned int *P = malloc(sizeof(unsigned int));
		ConvertToPosDat(1, x, y, P);
		SetDataFlag(floorTile, FLAG_POS, P);
		addEntToContainer(map, floorTile);
	}
	ENTITY *playerEnt;
	CreateEntity(&playerEnt);
	SetDataFlag(playerEnt, FLAG_NAME, "Shithead");
	B_PIXEL *PX = malloc(sizeof(PX));
	PX->text = '@';
	PX->color = 31;
	SetDataFlag(playerEnt, FLAG_APPEARANCE, PX);
	unsigned int *PlayerPos = malloc(sizeof(unsigned int));
	*PlayerPos = 0xFF000000;
	SetDataFlag(playerEnt, FLAG_POS, (void*)PlayerPos);
	SetDataFlag(game, FLAG_PLAYER, playerEnt);
	addEntToContainer(map, playerEnt);
	B_BUFFER *buffer;
	b_initialize(&buffer);
	SetDataFlag(game, FLAG_APPEARANCE, buffer);
	*out = game;
}