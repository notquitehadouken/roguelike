// Deleted and rewritten, August 30
#pragma once

int __UID = 0;

enum __ENTDATAFLAGS {
	FLAG_PLACE = 0, // current life stage (int)
	FLAG_NAME, // names of entities, player name for games (*char)
	FLAG_APPEARANCE, // screens for games, colors for objects (*char[2])
	FLAG_POS, // positions for entities (0xZZXXXYYY)
	// X and Y are actual position, Z is used for rendering order
	// Z does not change
	// X = (POS >> 12) & 0xFFF, Y = POS & 0xFFF, Z = (POS >> 24) & 0xFF
	FLAG_HEALTH, // (int)
	FLAG_MAXHEALTH, // (int)
	FLAG_CONTAINER, // entities this entity contains (**ENTITY)
	FLAG_CONTAINEDBY, // reverse of FLAG_CONTAINER (*ENTITY)
	FLAG_MAPS, // map list for games (**ENTITY)
	FLAG_CONTROLS, // controller for entities (*void)
	FLAG_PLAYER, // current player entity in a game (*ENTITY)
};

enum __ENTBOOLFLAGS {
	BFLAG_INIT = 0, // this entity is initialized
	BFLAG_COLLIDABLE, // this entity is collided with when walking
	BFLAG_DESTRUCTIBLE, // this entity is destroyed when its health hits 0
	BFLAG_INVISIBLE, // this entity is invisible to you and ai
	BFLAG_NORENDER, // do not render this entity
};

struct __ENT{
	void *data[64];
	unsigned long long dataflag;
	unsigned long long boolflag;
	int uid;
	char destroyed;
};

typedef struct __ENT ENTITY;

inline void ClearDataFlag(ENTITY *E, int flag)
{
	E->dataflag &= ~(1 << flag);
	free(E->data[flag]);
}

inline void SetDataFlag(ENTITY *E, int flag, void *value)
{
	ClearDataFlag(E, flag); // memory leak (tm)
	E->dataflag = E->dataflag | (1 << flag);
	E->data[flag] = value; // Hope you malloc'd.;
}

inline void GetDataFlag(const ENTITY *E, int flag, void **out)
{
	if (E->dataflag & (1 << flag))
		*out = E->data[flag];
	else
	{
		puts("flag no exist lamao");
		*out = 0;
	}
}

inline void ClearBoolFlag(ENTITY *E, int flag)
{
	E->boolflag &= ~(1 << flag);
}

inline void SetBoolFlag(ENTITY *E, int flag)
{
	E->boolflag |= 1 << flag;
}

inline void GetBoolFlag(const ENTITY *E, int flag, char *out)
{
	*out = E->boolflag & 1 << flag;
}

inline void DestroyEntity(ENTITY *E)
{
	for (int i = 0; i < 64; i++)
	{
		ClearDataFlag(E, i);
	}
	E->dataflag = 0;
	E->boolflag = 0;
	E->destroyed = 1;
	free(E);
}

inline void CreateEntity(ENTITY **out) // creates an entity
{
	ENTITY *E = malloc(sizeof(ENTITY));
	E->destroyed = 0;
	E->dataflag = 0;
	E->boolflag = 0;
	E->uid = ++__UID;
	*out = E;
}