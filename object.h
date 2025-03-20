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
    FLAG_CONTROLS, // controller for entities (void). the player lacks a controller.
    FLAG_PLAYER, // current player entity in a game (ENTITY*)
    FLAG_HEALTH, // health and maximum health (int[2]*)
    FLAG_CHANGE_HP_ON_STEP, // int*
    FLAG_SPEED, // normal speed is 256. lower values mean slower (int*)
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

enum ENTCONTROLLERTYPE {
    CONT_NOACTION = 0, // The entity will never do anything
    CONT_DEAD, // Equivalent to CONT_NOACTION.
    CONT_MOVETOPLAYER, // Will walk in a straight line towards the player
};

struct CONTROLLER {
    ENTITY *associate;
    int type;
    unsigned long long nextAct;
};

typedef struct CONTROLLER ENTITY_CONTROLLER;

extern ENTITY_CONTROLLER *CreateController(const int type) {
    ENTITY_CONTROLLER *PCont = malloc(sizeof(ENTITY_CONTROLLER*));
    PCont->associate = 0;
    PCont->type = type;
    return PCont;
}

extern void ConvertToZXY(const unsigned int position, int *Z, int *X, int *Y) {
    *Z = (position >> 24) & 0xFF;
    *X = (position >> 12) & 0xFFF;
    *Y = position & 0xFFF;
}

/**
 * Converts Z, X
 * @param Z
 * @param X
 * @param Y
 * @param out
 */
extern void ConvertToPosDat(const int Z, const int X, const int Y, unsigned int *out) {
    *out = Z << (4 * 6) | X << (4 * 3) | Y;
}

/**
 * Checks if an entity has a data flag
 * @param E The entity
 * @param flag The FLAG flag
 */
extern char HasDataFlag(const ENTITY* E, const int flag) {
    return (E->dataflag & (1 << flag)) != 0;
}

/**
 * Removes a data flag on an entity
 * @param E The entity
 * @param flag The FLAG flag
 */
extern void ClearDataFlag(ENTITY *E, const int flag) {
    if (!HasDataFlag(E, flag))
        return;
    E->dataflag &= ~(1 << flag);
    free(E->data[flag]);
}

/**
 * Sets a data flag on an entity
 * @param E The entity
 * @param flag The FLAG flag
 * @param value The value
 */
extern void SetDataFlag(ENTITY *E, const int flag, void *value) {
    ClearDataFlag(E, flag); // memory leak (tm)
    E->dataflag = E->dataflag | (1 << flag);
    E->data[flag] = value; // Hope you malloc'd.;
}

/**
 * Gets a data flag on an entity
 * @param E The entity
 * @param flag The FLAG flag
 * @param out The output
 */
extern void GetDataFlag(const ENTITY *E, const int flag, void **out) {
    if (E->dataflag & (1 << flag))
        *out = E->data[flag];
    else
        *out = 0;
}

/**
 * Clears a bool flag on an entity
 * @param E The entity
 * @param flag The BFLAG flag
 */
extern void ClearBoolFlag(ENTITY *E, const int flag) {
    E->boolflag &= ~(1 << flag);
}

/**
 * Sets a bool flag on an entity
 * @param E The entity
 * @param flag The BFLAG flag
 */
extern void SetBoolFlag(ENTITY *E, const int flag) {
    E->boolflag |= 1 << flag;
}


/**
 * Gets if an entity has a certain boolean flag
 * @param E The entity
 * @param flag The BFLAG flag
 * @param out The value of the flag.
 */
extern void GetBoolFlag(const ENTITY *E, const int flag, char *out) {
    *out = E->boolflag & 1 << flag;
}

/**
 * Checks if an entity has a certain boolean flag
 * @param E The entity
 * @param flag The BFLAG flag
 * @return If it does
 */
extern char HasBoolFlag(const ENTITY *E, const int flag) {
    return (E->boolflag & 1 << flag) != 0;
}

/**
 * Destroys an entity. The entity is not freed to prevent issues with entity lookups.
 * @param E The entity
 */
extern void DestroyEntity(ENTITY *E) {
    for (int i = 0; i < MAX_STR_LEN; i++) {
        ClearDataFlag(E, i);
    }
    E->dataflag = 0;
    E->boolflag = 0;
    E->destroyed = 1;
    //free(E); //Entity is not freed.
}

ENTITY **UID_LOOKUP = 0;
int UID_BLOCKS = 0;
#define UID_BLOCKSIZE 65536

/**
 * Creates an entity and initializes all its values
 * @param out The entity
 */
extern void CreateEntity(ENTITY **out) { // creates an entity
    ENTITY *E = malloc(sizeof(ENTITY));
    E->destroyed = 0;
    E->dataflag = 0;
    E->boolflag = 1;
    const int thisUid = ++GLOBAL_UID;
    E->uid = thisUid;
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
    if (!uid || // "Zero" is not an entity
        uid > UID_BLOCKS * UID_BLOCKSIZE || // Memory fault
        uid > GLOBAL_UID) // Time travel has not been implemented
        return 0;

    return UID_LOOKUP[uid];
}

/**
 * Gets every entity on a certain position in a map
 * @param MAP The map
 * @param X The X position
 * @param Y The Y position
 * @param out The return value, a pointer to a list of entities
 * @param count The amount of entities in *out
 * @return If the operation succeeded
 */
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

/**
 * Adds an basic entity controller to an entity
 * @param E The entity
 * @param Controller The controller
 */
extern void AddController(ENTITY *E, ENTITY_CONTROLLER *Controller) {
    if (!Controller) {
        Controller = CreateController(CONT_NOACTION);
    }
    SetDataFlag(E, FLAG_CONTROLS, Controller);
    Controller->associate = E;
}

/**
 * The amount of time passed, counted in 1/256 second increments
 */
unsigned long long GLOBAL_TIMER = 0; // 256 is considered "One second"

/**
 * Scales a time value to an entity's speed
 * @param Time The amount of time
 * @param E The entity
 * @return The scaled time value
 */
int ScaleTime(unsigned long long Time, ENTITY *E) {
    if (Time == 0 || !E)
        return Time;
    int *s;
    GetDataFlag(E, FLAG_SPEED, (void**)&s);
    if (!s)
        return Time;
    Time *= 256;
    Time /= *s;
    return Time;
}

/**
 * Advances time. Only used by the player.
 * @param Time The amount of time
 * @param E The player
 * @return If the time value is non-zero.
 */
int GTimeAdvance(const unsigned long long Time, ENTITY *E) {
    if (!Time)
        return 0;
    GLOBAL_TIMER += ScaleTime(Time, E);
    return 1;
}
