// Deleted and rewritten, August 30
#pragma once
#include "gamestate.h"
#include "global.h"

typedef int UID__TYPE;
UID__TYPE GLOBAL_UID = 0;
unsigned long long GLOBAL_TIMER = 0; // 100 is considered "One second"

enum ENTDATAFLAGS
{
  FLAG_PLACE = 0, // current life stage (int)
  FLAG_NAME, // names of entities, player name for games (char*)
  FLAG_APPEARANCE, // screens for games, colors for objects (B_PIXEL*)
  FLAG_PLAYER, // current player entity in a game (ENTITY**)
  FLAG_POS, // positions for entities (0xZZXXXYYY, int*)
  FLAG_CONTAINER, // entities this entity contains (_CACHE_OF(ENTITY)*)
  FLAG_CONTAINEDBY, // reverse of FLAG_CONTAINER (ENTITY**)
  FLAG_ZVEL, // upward velocity (int*)
  FLAG_LEVITATE, // how high above the ground you can stay (int*)
  FLAG_CANJUMP, // how high you can jump (int*)
  FLAG_MAPS, // map list for games (_CACHE_OF(ENTITY)*)
  FLAG_CONTROLS, // controller for entities (ENTITY_CONTROLLER*). the player lacks a controller.
  FLAG_HEALTH, // health and maximum health (int[2]*)
  FLAG_ARMOR, // flat reduction on damage (int*)
  FLAG_SPEED, // normal speed is 100. lower values mean faster (int*)
  FLAG_SIGHTRANGE, // how far you can see (int*)
  FLAG_FRAGILE, // damage this thing takes when moved into horizontally (int*)
  FLAG_TOP, // amount of flags
};

enum ENTBOOLFLAGS
{
  BFLAG_INIT = 0, // this entity is initialized
  BFLAG_ISMAP, // this entity is a map
  BFLAG_STOPPING, // this entity stops movement
  BFLAG_STATIC, // this entity does not fall
  BFLAG_CLIMBER, // this entity can climb walls
  BFLAG_DESTRUCTIBLE, // this entity is destroyed when its health hits 0
  BFLAG_INVISIBLE, // this entity is invisible to you and ai
  BFLAG_NORENDER, // do not render this entity
  BFLAG_OCCLUDING, // this entity blocks light
  BFLAG_PUSHABLE, // this entity can be pushed by movers
  BFLAG_TOP, // amount of bflags
};

typedef unsigned long long FLAG__TYPE;
#define FLAG_COUNT (sizeof(FLAG__TYPE)*8)

struct ENT
{
  void* data[FLAG_COUNT];
  FLAG__TYPE dataflag;
  FLAG__TYPE boolflag;
  UID__TYPE uid;
  char destroyed;
};

typedef struct ENT ENTITY;

enum ENTCONTROLLERTYPE
{
  CONT_NOACTION = 0, // The entity will never do anything
  CONT_DEAD, // Equivalent to CONT_NOACTION
  CONT_ZVEL_HANDLE, // This entity makes shit fall
  CONT_MOVETOPLAYER, // Will walk in a straight line towards the player
  CONT_DOOR, // Opens when something with BFLAG_DOOROPEN is near
};

struct CONTROLLER
{
  ENTITY* associate;
  int type;
  unsigned long long nextAct;
};

typedef struct CONTROLLER ENTITY_CONTROLLER;

ENTITY** UID_LOOKUP = 0;
int UID_BLOCKS = 0;
#define UID_BLOCKSIZE 65536

/**
 * Creates an entity and initializes all its values
 * @param out The entity
 */
extern void CreateEntity(ENTITY** out)
{
  // creates an entity
  ENTITY* E = malloc(sizeof(ENTITY));
  E->destroyed = 0;
  E->dataflag = 0;
  E->boolflag = 1;
  const int thisUid = ++GLOBAL_UID;
  E->uid = thisUid;
  if (thisUid >= UID_BLOCKS * UID_BLOCKSIZE)
  {
    if (UID_LOOKUP)
    {
      ENTITY** NEW_LOOKUP = calloc(UID_BLOCKSIZE * (UID_BLOCKS + 1), sizeof(ENTITY*));
      int i;
      for (i = 0; i < UID_BLOCKSIZE * UID_BLOCKS; i++)
      {
        NEW_LOOKUP[i] = UID_LOOKUP[i];
      }
      free(UID_LOOKUP);
      UID_LOOKUP = NEW_LOOKUP;
    }
    else
    {
      UID_LOOKUP = calloc(UID_BLOCKSIZE, sizeof(ENTITY*));
    }
    UID_BLOCKS++;
  }
  UID_LOOKUP[thisUid] = E;
  int i;
  for (i = 0; i < sizeof(unsigned long long) * 8; i++)
  {
    E->data[i] = 0;
  }
  *out = E;
}

extern attrpure ENTITY* EntityLookup(const UID__TYPE uid)
{
  // finds an entity by uid
  if (!uid || // "Zero" is not an entity
    uid > UID_BLOCKS * UID_BLOCKSIZE || // Memory fault
    uid > GLOBAL_UID) // Time travel has not been implemented
      return 0;

  return UID_LOOKUP[uid];
}

extern attrpure ENTITY_CONTROLLER* CreateController(const int type)
{
  ENTITY_CONTROLLER* PCont = malloc(sizeof(ENTITY_CONTROLLER));
  PCont->associate = 0;
  PCont->type = type;
  PCont->nextAct = GLOBAL_TIMER;
  return PCont;
}

extern void ConvertToZXY(const uint position, int* Z, int* X, int* Y)
{
  *Z = (position >> 24) & 0xFF;
  *X = (position >> 12) & 0xFFF;
  *Y = position & 0xFFF;
}

/**
 * Converts Z, X, and Y to a single integer representing a position
 * @param Z
 * @param X
 * @param Y
 * @param out
 */
extern void ConvertToPosDat(const int Z, const int X, const int Y, uint* out)
{
  *out = Z << (4 * 6) | X << (4 * 3) | Y;
}

#define InBounds(X, Y) (inRange(X, 0, MAP_WIDTH - 1) && inRange(Y, 0, MAP_HEIGHT - 1))
#define BoundedBy3D(X, Y, Z, X1, Y1, Z1, X2, Y2, Z2) (inRange(X, X1, X2) && inRange(Y, Y1, Y2) && inRange(Z, Z1, Z2))
#define InBounds3D(X, Y, Z) BoundedBy3D(X, Y, Z, 0, 0, 0, MAP_WIDTH - 1, MAP_HEIGHT - 1, MAP_DEPTH - 1)

/**
 * Checks if an entity has a data flag
 * @param E The entity
 * @param flag The FLAG flag
 */
extern attrpure char HasDataFlag(const ENTITY* E, const int flag)
{
  return (E->dataflag & (1 << flag)) != 0;
}

/**
 * Removes a data flag on an entity
 * @param E The entity
 * @param flag The FLAG flag
 */
extern void ClearDataFlag(ENTITY* E, const int flag)
{
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
extern void SetDataFlag(ENTITY* E, const int flag, void* value)
{
  ClearDataFlag(E, flag); // memory leak (tm)
  E->dataflag = E->dataflag | (1 << flag);
  E->data[flag] = value; // Hope you malloc'd.
}

/**
 * Gets a data flag on an entity
 * @param E The entity
 * @param flag The FLAG flag
 * @param out The output
 */
extern void GetDataFlag(const ENTITY* E, const int flag, void** out)
{
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
extern void ClearBoolFlag(ENTITY* E, const int flag)
{
  E->boolflag &= ~(1 << flag);
}

/**
 * Sets a bool flag on an entity
 * @param E The entity
 * @param flag The BFLAG flag
 */
extern void SetBoolFlag(ENTITY* E, const int flag)
{
  E->boolflag |= 1 << flag;
}


/**
 * Gets if an entity has a certain boolean flag
 * @param E The entity
 * @param flag The BFLAG flag
 * @param out The value of the flag.
 */
extern void GetBoolFlag(const ENTITY* E, const int flag, char* out)
{
  *out = E->boolflag & 1 << flag;
}

/**
 * Checks if an entity has a certain boolean flag
 * @param E The entity
 * @param flag The BFLAG flag
 * @return If it does
 */
extern attrpure char HasBoolFlag(const ENTITY* E, const int flag)
{
  return (E->boolflag & 1 << flag) != 0;
}

extern attrpure char *ToPrettyString(const ENTITY* Target)
{
  char *Return = calloc(MAX_STR_LEN * 2, sizeof(char));
  char *Name;
  GetDataFlag(Target, FLAG_NAME, (void**)&Name);
  if (Name)
  {
    sprintf(Return, "%s (%d)", Name, Target->uid);
  }
  else
  {
    sprintf(Return, "%d", Target->uid);
  }

  return Return;
}

_CACHE_OF(ENTITY)* POSITION_CACHE;

extern void PopulateEntityCacheArray(_CACHE_OF(ENTITY)** Cache, const uint DesiredCount)
{
  _CACHE_OF(ENTITY)* Target = calloc(DesiredCount, sizeof(*Target));
  int i;
  for (i = 0; i < DesiredCount; i++)
  {
    cacheInit(_CACHE_REF(Target + i));
  }
  *Cache = Target;
}

extern void PopulateCacheArrays()
{
  PopulateEntityCacheArray(&POSITION_CACHE, MAP_CELLCOUNT);
}

/**
 * Gets an entity's Z position. -1 if the entity has no position.
 * Ranges from 0 - MAP_DEPTH-1 on success.
 */
extern attrpure int GetEntZ(const ENTITY *E)
{
  if (!HasDataFlag(E, FLAG_POS))
    return -1;
  int Z, u;
  uint *CPos;
  GetDataFlag(E, FLAG_POS, (void**)&CPos);
  ConvertToZXY(*CPos, &Z, &u, &u);
  return Z;
}

/**
 * Sets an entity's Z position.
 */
extern void SetEntZ(ENTITY *E, const int Z)
{
  int X;
  int Y;
  uint NPos;
  uint *CPos;
  GetDataFlag(E, FLAG_POS, (void**)&CPos);
  ConvertToZXY(*CPos, &X, &X, &Y);
  ConvertToPosDat(Z, X, Y, &NPos);
  SetDataFlag(E, FLAG_POS, uintap(NPos));
}

/**
 * Gets all entities with a given X and Y, regardless of map.
 */
extern attrmalloc ENTITY** GetEntsAtPos(const uint X, const uint Y)
{
  if (!InBounds(X, Y))
    return 0;
  return (ENTITY**)cacheAsList(_CACHE(POSITION_CACHE[MAP_IND(X, Y)]));
}

/**
 * Update entity position properly.
 * Entity must already have a position.
 */
extern void MoveEnt(ENTITY *E, const int X, const int Y)
{
  uint *CPos;
  uint NPos;
  GetDataFlag(E, FLAG_POS, (void**)&CPos);
  int Z, oX, oY;
  ConvertToZXY(*CPos, &Z, &oX, &oY);
  ConvertToPosDat(Z, X, Y, &NPos);
  SetDataFlag(E, FLAG_POS, uintap(NPos));
  if (InBounds(oX, oY))
  {
    cacheRemove(_CACHE(POSITION_CACHE[MAP_IND(oX, oY)]), E);
  }
  if (InBounds(X, Y))
  {
    cacheAdd(_CACHE_REF(POSITION_CACHE + MAP_IND(X, Y)), E);
  }
}

extern void SetEntPos(ENTITY *E, const int X, const int Y, const int Z)
{
  if (HasDataFlag(E, FLAG_POS))
  {
    MoveEnt(E, X, Y);
    SetEntZ(E, Z);
    return;
  }
  uint Pos;
  ConvertToPosDat(Z, X, Y, &Pos);
  SetDataFlag(E, FLAG_POS, uintap(Pos));
  if (InBounds(X, Y))
  {
    cacheAdd((void****)(POSITION_CACHE + MAP_IND(X, Y)), E);
  }
}

/**
 * Destroys an entity. The entity is not freed to prevent issues with entity lookups.
 * @param E The entity
 */
extern void DeleteEntity(ENTITY* E)
{
  int i;
  for (i = 0; i < FLAG_COUNT; i++)
  {
    ClearDataFlag(E, i);
  }
  E->dataflag = 0;
  E->boolflag = 0;
  E->destroyed = 1;
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
extern char GetEntitiesOnPosition(const ENTITY* MAP, const int X, const int Y, ENTITY*** out, int* count)
{
  if (!InBounds(X, Y))
    return 0;

  ENTITY** ELIST = GetEntsAtPos(X, Y);
  if (!ELIST)
    return 0;

  if (!(ELIST[0])) // No entities are on that position
  {
    *out = calloc(1, sizeof(ENTITY*));
    *count = 0;
    free(ELIST);
    return 1;
  }

  uint elistLen = 0;

  while (ELIST[elistLen++]);
  ENTITY** tList = calloc(elistLen, sizeof(ENTITY*));

  *count = 0;
  int i;
  for (i = 0; ELIST[i]; i++) // ELIST is null-terminated.
  {
    ENTITY** ContainingEntity;
    GetDataFlag(ELIST[i], FLAG_CONTAINEDBY, (void**)&ContainingEntity);
    if (ContainingEntity && *ContainingEntity == MAP)
    {
      tList[(*count)++] = ELIST[i];
    }
  }

  *out = calloc(*count, sizeof(ENTITY*));

  for (i = 0; i < *count; i++) (*out)[i] = tList[i];

  free(tList);
  free(ELIST);
  return 1;
}

/**
 * Adds an basic entity controller to an entity
 * @param E The entity
 * @param Controller The controller
 */
extern void AddController(ENTITY* E, ENTITY_CONTROLLER* Controller)
{
  if (!Controller)
  {
    Controller = CreateController(CONT_NOACTION);
  }
  SetDataFlag(E, FLAG_CONTROLS, Controller);
  Controller->associate = E;
}

/**
 * Scales a time value to an entity's speed
 * @param Time The amount of time
 * @param E The entity
 * @return The scaled time value
 */
int attrpure ScaleTime(int Time, const ENTITY* E)
{
  if (Time == 0 || !E)
    return Time;
  int* s;
  GetDataFlag(E, FLAG_SPEED, (void**)&s);
  if (!s)
    return Time;
  Time *= *s;
  Time /= 100;
  return Time;
}

/**
 * Advances time. Only used by the player.
 * @param Time The amount of time
 * @return If the time value is non-zero.
 */
int GTimeAdvance(const int Time)
{
  if (!Time)
    return 0;
  GLOBAL_TIMER += Time;
  return 1;
}
