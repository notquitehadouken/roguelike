// Deleted and rewritten, August 30
#pragma once
#include "gamestate.h"
#define CONTAINERCAPACITY 65536
#define GAMETOTALMAPS 256

int GLOBAL_UID = 0;

enum ENTDATAFLAGS {
    FLAG_PLACE = 0, // current life stage (int)
    FLAG_NAME, // names of entities, player name for games (char*)
    FLAG_APPEARANCE, // screens for games, colors for objects (char[2]*)
    FLAG_POS, // positions for entities (0xZZXXXYYY, int*)
    FLAG_CONTAINER, // entities this entity contains (ENTITY**)
    FLAG_CONTAINEDBY, // reverse of FLAG_CONTAINER (ENTITY*)
    FLAG_MAPS, // map list for games (ENTITY**)
    FLAG_CONTROLS, // controller for entities (void)
    FLAG_PLAYER, // current player entity in a game (ENTITY*)
    FLAG_HEALTH, // health and maximum health (int[2]*)
    FLAG_CHANGE_HP_ON_STEP, // int*
};

enum ENTBOOLFLAGS {
    BFLAG_INIT = 0, // this entity is initialized
    BFLAG_ISMAP, // this entity is a map
    BFLAG_COLLIDABLE, // this entity is collided with when walking
    BFLAG_DESTRUCTIBLE, // this entity is destroyed when its health hits 0
    BFLAG_INVISIBLE, // this entity is invisible to you and ai
    BFLAG_NORENDER, // do not render this entity
    BFLAG_OCCLUDING, // this entity blocks light
};

struct ENT{
    void *data[sizeof(unsigned long long) * 8];
    unsigned long long dataflag;
    unsigned long long boolflag;
    int uid;
    char destroyed;
};

typedef struct ENT ENTITY;

struct CONTROLLER {

};

typedef struct CONTROLLER ENTITY_CONTROLLER;

extern void ConvertToZXY(const unsigned int position, int *Z, int *X, int *Y) {
    *Z = (position >> 24) & 0xFF;
    *X = (position >> 12) & 0xFFF;
    *Y = position & 0xFFF;
}

extern void ConvertToPosDat(const int Z, const int X, const int Y, unsigned int *out) {
    *out = Z << (4 * 6) | X << (4 * 3) | Y;
}

extern char HasDataFlag(const ENTITY* E, const int flag) {
    return (E->dataflag & (1 << flag)) != 0;
}

extern void ClearDataFlag(ENTITY *E, const int flag) {
    if (!HasDataFlag(E, flag))
        return;
    E->dataflag &= ~(1 << flag);
    free(E->data[flag]);
}

extern void SetDataFlag(ENTITY *E, const int flag, void *value) {
    ClearDataFlag(E, flag); // memory leak (tm)
    E->dataflag = E->dataflag | (1 << flag);
    E->data[flag] = value; // Hope you malloc'd.;
}

extern void GetDataFlag(const ENTITY *E, const int flag, void **out) {
    if (E->dataflag & (1 << flag))
        *out = E->data[flag];
    else
        *out = 0;
}

extern void ClearBoolFlag(ENTITY *E, const int flag) {
    E->boolflag &= ~(1 << flag);
}

extern void SetBoolFlag(ENTITY *E, const int flag) {
    E->boolflag |= 1 << flag;
}

extern void GetBoolFlag(const ENTITY *E, const int flag, char *out) {
    *out = E->boolflag & 1 << flag;
}

extern char HasBoolFlag(const ENTITY *E, const int flag) {
    return (E->boolflag & 1 << flag) != 0;
}

extern void DestroyEntity(ENTITY *E) {
    for (int i = 0; i < MAX_STR_LEN; i++) {
        ClearDataFlag(E, i);
    }
    E->dataflag = 0;
    E->boolflag = 0;
    E->destroyed = 1;
    free(E);
}

ENTITY **UID_LOOKUP = 0;
int UID_BLOCKS = 0;
#define UID_BLOCKSIZE 65536

extern void CreateEntity(ENTITY **out) { // creates an entity
    ENTITY *E = malloc(sizeof(ENTITY));
    E->destroyed = 0;
    E->dataflag = 0;
    E->boolflag = 1;
    const int thisUid = ++GLOBAL_UID;
    E->uid = ++GLOBAL_UID;
    if (thisUid >= UID_BLOCKS * UID_BLOCKSIZE) {
        if (UID_LOOKUP) {
            ENTITY **NEW_LOOKUP = calloc(UID_BLOCKSIZE * (UID_BLOCKS + 1), sizeof(ENTITY*));
            for (int i = 0; i < UID_BLOCKSIZE * UID_BLOCKS; i++) {
                NEW_LOOKUP[i] = UID_LOOKUP[i];
            }
            free(UID_LOOKUP);
            UID_LOOKUP = NEW_LOOKUP;
        }
        else {
            UID_LOOKUP = calloc(UID_BLOCKSIZE, sizeof(ENTITY*));
        }
        UID_BLOCKS++;
    }
    UID_LOOKUP[thisUid] = E;
    for (int i = 0; i < sizeof(unsigned long long) * 8; i++) {
        E->data[i] = 0;
    }
    *out = E;
}

extern ENTITY *EntityLookup(const int uid) { // finds an entity by uid
    if (!uid)
        return 0;

}

extern char GetEntitiesOnPosition(const ENTITY *MAP, const int X, const int Y, ENTITY ***out, int *count) {
    ENTITY **ELIST;
    GetDataFlag(MAP, FLAG_CONTAINER, (void**)&ELIST);
    if (!ELIST)
        return 0;
    ENTITY **list = malloc(CONTAINERCAPACITY * sizeof(ENTITY*));
    *count = 0;
    for (int i = 0; i < CONTAINERCAPACITY; i++) {
        if (!ELIST[i])
    		break;
		unsigned int *pos;
		GetDataFlag(ELIST[i], FLAG_POS, (void**)&pos);
		int _, x, y;
		ConvertToZXY(*pos, &_, &x, &y);
		if (x == X && y == Y) {
			list[(*count)++] = ELIST[i];
		}
	}
    ENTITY **nList = calloc(*count, sizeof(ENTITY*));
    for (int i = 0; i < *count; i++) {
        nList[i] = list[i];
    }
    *out = nList;
    free(list);
    return 1;
}

extern char InBounds(const int X, const int Y) {
  return X >= 0 && Y >= 0 && X < MAP_WIDTH && Y < MAP_HEIGHT;
}