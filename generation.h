#pragma once

unsigned long long RANDOM_SEED = 0x8F52B09AA4510DC5;

extern void random_scramble(long long variantize) {
    RANDOM_SEED = ~RANDOM_SEED;
    RANDOM_SEED ^= variantize;
    RANDOM_SEED ^= RANDOM_SEED >> sizeof(RANDOM_SEED) * 4 - 1;
    RANDOM_SEED ^= RANDOM_SEED << sizeof(RANDOM_SEED) * 3;
    RANDOM_SEED ^= (long long)&variantize << sizeof(RANDOM_SEED) * 2;
}

extern int random_nextInt() {
    random_scramble(0x1234fedc5678ba98);
    return RANDOM_SEED;
}

extern int **createLine(const int x, const int y, int *outCount) { // Draws a line from 0, 0
	if (x > 0 && y > 0 && x <= y) {
		int **all = calloc(x + 2, sizeof(int*));
		int D = 2 * x - y;
		int Y = 0;
		for (int i = 0; i < x; i++) {
			int *p = calloc(2, sizeof(int));
			all[i] = p;
			p[0] = x;
			p[1] = Y;
			if (D > 0) {
				Y++;
				D -= 2 * x;
			}
			D += 2 * y;
		}
		return all;
	}
	if (x == 0 && y == 0) {
		int **all = malloc(sizeof(int*));
		all[0] = calloc(2, sizeof(int));
		all[0][0] = 0;
		all[0][1] = 0;
		*outCount = 1;
		return all;
	}
	if (x < 0) {
		int **all = createLine(-x, y, outCount);
		for (int i = 0; i < *outCount; i++) {
			all[i][0] *= -1;
		}
		return all;
	}
	if (y < 0) {
		int **all = createLine(x, -y, outCount);
		for (int i = 0; i < *outCount; i++) {
			all[i][1] *= -1;
		}
		return all;
	}
	if (x > y) {
		int **all = createLine(y, x, outCount);
		for (int i = 0; i < *outCount; i++) {
			const int t = all[i][0];
			all[i][0] = all[i][1];
			all[i][1] = t;
		}
		return all;
	}
	return 0; // You've broken something
}

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
			RANDOM_SEED = ((RANDOM_SEED & 0b1) << (sizeof(RANDOM_SEED) * 8 - 1)) ^ RANDOM_SEED >> 1;
			RANDOM_SEED ^= (long long)floorPixel;
			RANDOM_SEED = (RANDOM_SEED >> sizeof(RANDOM_SEED) * 2) ^ (RANDOM_SEED << 11);
			RANDOM_SEED ^= ~(long long)floorTile;
			if (random_nextInt() % 10 == 9) {
				floorPixel->color = 91;
				int *HC = malloc(sizeof(int));
				*HC = -1;
				SetDataFlag(floorTile, FLAG_CHANGE_HP_ON_STEP, HC);
			}
			else if (random_nextInt() % 15 == 1) {
				floorPixel->color = 92;
				int *HC = malloc(sizeof(int));
				*HC = 1;
				SetDataFlag(floorTile, FLAG_CHANGE_HP_ON_STEP, HC);
			}
			if (x > 3 && y > 3 && !(RANDOM_SEED & 0b111)) {
				ENTITY *wall;
				CreateEntity(&wall);
				B_PIXEL *wallPixel = malloc(sizeof(wallPixel));
				wallPixel->text = '#';
				wallPixel->color = 39;
				SetDataFlag(wall, FLAG_APPEARANCE, wallPixel);
				unsigned int *WALLP = malloc(sizeof(unsigned int));
				ConvertToPosDat(2, x, y, WALLP);
				SetDataFlag(wall, FLAG_POS, WALLP);
				SetBoolFlag(wall, BFLAG_COLLIDABLE);
				addEntToContainer(map, wall);
			}
		}

	ENTITY *playerEnt;
	CreateEntity(&playerEnt);
	SetDataFlag(playerEnt, FLAG_NAME, "Player");

	B_PIXEL *PX = malloc(sizeof(PX));
	PX->text = '@';
	PX->color = 31;
	SetDataFlag(playerEnt, FLAG_APPEARANCE, PX);

	unsigned int *PlayerPos = malloc(sizeof(unsigned int));
	*PlayerPos = 0xFF000000;
	SetDataFlag(playerEnt, FLAG_POS, PlayerPos);

	int *HP = malloc(2 * sizeof(int));
	HP[0] = 100;
	HP[1] = 150;
	SetDataFlag(playerEnt, FLAG_HEALTH, HP);

	SetDataFlag(game, FLAG_PLAYER, playerEnt);
	addEntToContainer(map, playerEnt);
	B_BUFFER *buffer;
	b_initialize(&buffer);
	SetDataFlag(game, FLAG_APPEARANCE, buffer);
	*out = game;
}