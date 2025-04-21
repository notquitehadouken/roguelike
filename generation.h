#pragma once

/**
 * Adds an entity to a container
 * @param container The container
 * @param ent The entity
 */
extern void addEntToContainer(ENTITY *container, ENTITY *ent) {
    ENTITY **ELIST; // (-1) [cursed]
    GetDataFlag(container, FLAG_CONTAINER, (void**)&ELIST);
    for (int i = 0; i < CONTAINERCAPACITY /* If it hits CONTAINERCAPACITY entities,
        you are fucked.*/; i++) {
        if (!ELIST[i]) {
        	ENTITY **containerRef = malloc(sizeof(ENTITY*));
        	*containerRef = container;
            SetDataFlag(ent, FLAG_CONTAINEDBY, containerRef);
            ELIST[i] = ent;
            return;
        }
    }
}

/**
 * Removes an entity from a container
 * The calling function is expected to find a new place for the entity
 * @param container The container
 * @param ent The entity
 */
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

/**
 * Adds a container to an entity if one does not exist already
 * @param E The entity
 */
extern void addContainer(ENTITY *E) {
    if (HasDataFlag(E, FLAG_CONTAINER))
        return;
    void **LIST = malloc(CONTAINERCAPACITY * sizeof(int*));
    for (int i = 0; i < CONTAINERCAPACITY; i++)
        LIST[i] = 0;
    SetDataFlag(E, FLAG_CONTAINER, LIST);
}

/**
 * Removes an entity's container if it has one
 * Every entity in that container is automatically placed into the parent container of E
 * They also inherit E's position, assuming they have one.
 * @param E The entity that will lose its container
 */
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

/**
 * Generates an entity with most required parameters
 * @param In The entity it should be spawned inside of
 * @param X X position
 * @param Y Y position
 * @param Z Z rendering order
 * @param Text The text of the entity
 * @param Color The color of the entity
 * @return The entity
 */
extern ENTITY *entFactory(ENTITY *In, const int X, const int Y, const unsigned char Z, const char Text, const char Color) { // Creates an entity with all required data
	ENTITY *E;
	CreateEntity(&E);
	B_PIXEL *EP = malloc(sizeof(EP));
	EP->text = Text;
	EP->color = Color;
	SetDataFlag(E, FLAG_APPEARANCE, EP);
	unsigned int *P = malloc(sizeof(unsigned int));
	ConvertToPosDat(Z, X, Y, P);
	SetDataFlag(E, FLAG_POS, P);
	if (In)
		addEntToContainer(In, E);

	RANDOM_SEED = ((RANDOM_SEED & 0b1) << (sizeof(RANDOM_SEED) * 8 - 1)) ^ RANDOM_SEED >> 1;
	RANDOM_SEED ^= (long long)P;
	RANDOM_SEED = (RANDOM_SEED >> sizeof(RANDOM_SEED) * 2) ^ (RANDOM_SEED << 11);
	RANDOM_SEED ^= ~(long long)E;

	return E;
}

/**
 * Generates a game
 * @param out The return value
 */
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
			entFactory(map, x, y, 1, '.', 30);
			if (x >= 20 && y >= 20 && random_nextInt() % 30 == 1) {
				ENTITY *follower = entFactory(map, x, y, 3, '@', 33);
				SetBoolFlag(follower, BFLAG_COLLIDABLE);
				ENTITY_CONTROLLER *Controller = CreateController(CONT_MOVETOPLAYER);
				Controller->nextAct = GLOBAL_TIMER;
				AddController(follower, Controller);
				addEntToContainer(map, follower);
				int *S = malloc(sizeof(S));
				*S = 128;
				SetDataFlag(follower, FLAG_SPEED, S);
			}
			if (x == 10 && y == 10) {
				ENTITY *wall = entFactory(map, x, y, 2, '#', 39);
				SetBoolFlag(wall, BFLAG_COLLIDABLE);
				SetBoolFlag(wall, BFLAG_OCCLUDING);
				addEntToContainer(map, wall);
			}
		}

	ENTITY *playerEnt = entFactory(map, 0, 0, 255, '@', 31);
	SetDataFlag(playerEnt, FLAG_NAME, "Player");

	int *HP = malloc(2 * sizeof(int));
	HP[0] = 100;
	HP[1] = 150;
	SetDataFlag(playerEnt, FLAG_HEALTH, HP);
	SetDataFlag(game, FLAG_PLAYER, playerEnt);

	B_BUFFER *buffer;
	b_initialize(&buffer);
	SetDataFlag(game, FLAG_APPEARANCE, buffer);
	*out = game;
}