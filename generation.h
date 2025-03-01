#define MAPCAPACITY 4096
#define GAMETOTALMAPS 256

extern void createMap(ENTITY **out)
{
	ENTITY *E;
	
	CreateEntity(&E);
	void **LIST = malloc(MAPCAPACITY * sizeof(int*));
	for (int i = 0; i < MAPCAPACITY; i++)
		LIST[i] = 0;
	SetDataFlag(E, FLAG_CONTAINER, LIST);
	*out = E;
}

extern void addEntToMap(ENTITY *map, ENTITY *ent)
{
	ENTITY **ELIST; // (-1) [cursed]
	GetDataFlag(map, FLAG_CONTAINER, (void**)&ELIST);
	for (int i = 0; i < MAPCAPACITY /* If maps hit MAPCAPACITY entities,
		you are fucked.*/; i++)
	{
		if (!ELIST[i])
		{
			ELIST[i] = ent;
			return;
		}
	}
}

extern void removeEntFromMap(ENTITY *map, ENTITY *ent)
{
	ENTITY **ELIST; // (-1) [cursed]
	GetDataFlag(map, FLAG_CONTAINER, (void**)&ELIST);
	for (int i = 0; i < MAPCAPACITY; i++)
	{
		if (ELIST[i] == ent)
		{
			ELIST[i] = 0;
			return;
		}
	}
}

extern void generateGame(ENTITY **out)
{
	ENTITY *game;
	CreateEntity(&game);
	ENTITY *map;
	createMap(&map);
	ENTITY **maps = malloc(GAMETOTALMAPS * sizeof(map));
	maps[0] = map;
	SetDataFlag(game, FLAG_MAPS, maps);
	for (int i = 0; i < MAP_WIDTH*MAP_HEIGHT; i++){
		ENTITY *floorTile;
		CreateEntity(&floorTile);
		SetDataFlag(floorTile, FLAG_APPEARANCE, &DEFAULT_PIXEL);
	}
	ENTITY *playerEnt;
	CreateEntity(&playerEnt);
	SetDataFlag(playerEnt, FLAG_NAME, "Shithead");
	B_PIXEL PX;
	b_createPixel('@', 30, &PX);
	SetDataFlag(playerEnt, FLAG_APPEARANCE, &PX);
	SetDataFlag(game, FLAG_PLAYER, &playerEnt);
	B_BUFFER *buffer;
	b_initialize(&buffer);
	SetDataFlag(game, FLAG_APPEARANCE, buffer);
	*out = game;
}