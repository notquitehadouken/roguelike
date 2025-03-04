// Deleted and rewritten, August 30
#pragma once

int __UID = 0;

enum __ENTDATAFLAGS {
	FLAG_PLACE = 0, // current life stage (int)
	FLAG_NAME, // names of entities, player name for games (char*)
	FLAG_APPEARANCE, // screens for games, colors for objects (char[2]*)
	FLAG_POS, // positions for entities (0xZZXXXYYY)
	FLAG_CONTAINER, // entities this entity contains (ENTITY**)
	FLAG_CONTAINEDBY, // reverse of FLAG_CONTAINER (ENTITY*)
	FLAG_MAPS, // map list for games (ENTITY**)
	FLAG_CONTROLS, // controller for entities (void*)
	FLAG_PLAYER, // current player entity in a game (ENTITY*)
	FLAG_HEALTH, // health and maximum health (int[2]*)
};

enum __ENTBOOLFLAGS {
	BFLAG_INIT = 0, // this entity is initialized
	BFLAG_ISMAP, // this entity is a map
	BFLAG_COLLIDABLE, // this entity is collided with when walking
	BFLAG_DESTRUCTIBLE, // this entity is destroyed when its health hits 0
	BFLAG_INVISIBLE, // this entity is invisible to you and ai
	BFLAG_NORENDER, // do not render this entity
};

struct __ENT{
	void *data[sizeof(unsigned long long) * 8];
	unsigned long long dataflag;
	unsigned long long boolflag;
	int uid;
	char destroyed;
};

typedef struct __ENT ENTITY;

extern void ConvertToZXY(const unsigned int position, int *Z, int *X, int *Y) {
	*Z = (position >> 24) & 0xFFF;
	*X = (position >> 12) & 0xFFF;
	*Y = position & 0xFFF;
}

extern void ConvertToPosDat(const int Z, const int X, const int Y, unsigned int *out) {
	*out = Z << (4 * 6) | X << (4 * 3) | Y;
}

extern void ClearDataFlag(ENTITY *E, const int flag) {
	if (!(E->dataflag & (1 << flag)))
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

extern char HasDataFlag(const ENTITY* E, const int flag) {
	return (E->dataflag & (1 << flag)) != 0;
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

extern void CreateEntity(ENTITY **out) { // creates an entity
	ENTITY *E = malloc(sizeof(ENTITY));
	E->destroyed = 0;
	E->dataflag = 0;
	E->boolflag = 1;
	E->uid = ++__UID;
	for (int i = 0; i < sizeof(unsigned long long) * 8; i++) {
		E->data[i] = 0;
	}
	*out = E;
}