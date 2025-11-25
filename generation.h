#pragma once

#include "global.h"
#include "object.h"

enum ENT_PREFAB
{
  PREFAB_WALL = 0,
  PREFAB_WINDOW,
  PREFAB_PUSHBOX,
  PREFAB_FRAGILEWALL,
};

/**
 * Removes an entity from a container
 * The calling function is expected to find a new place for the entity
 * @param container The container
 * @param ent The entity
 */
extern void removeEntFromContainer(const ENTITY* container, ENTITY* ent)
{
  _CACHE_OF(ENTITY)* cache;
  GetDataFlag(container, FLAG_CONTAINER, (void**)&cache);
  cacheRemove(_CACHE(*cache), ent);
  ClearDataFlag(ent, FLAG_CONTAINEDBY);
}

/**
 * Adds an entity to a container
 * @param container The container
 * @param ent The entity
 */
extern void addEntToContainer(ENTITY* container, ENTITY* ent)
{
  if (HasDataFlag(ent, FLAG_CONTAINEDBY))
  {
    ENTITY** reference;
    GetDataFlag(ent, FLAG_CONTAINEDBY, (void**)&reference);
    removeEntFromContainer(*reference, ent);
  }
  _CACHE_OF(ENTITY)* cache;
  GetDataFlag(container, FLAG_CONTAINER, (void**)&cache);
  cacheAdd(_CACHE_REF(cache), ent);
  ENTITY** reference = malloc(sizeof(*reference));
  *reference = container;
  SetDataFlag(ent, FLAG_CONTAINEDBY, reference);
}

/**
 * Properly adds a container to an entity if one does not exist already
 * @param E The entity
 */
extern void addContainer(ENTITY* E)
{
  if (HasDataFlag(E, FLAG_CONTAINER))
    return;
  _CACHE_OF(ENTITY)* container = malloc(sizeof(*container));
  cacheInit(_CACHE_REF(container));
  SetDataFlag(E, FLAG_CONTAINER, container);
}

/**
 * Properly removes an entity's container if it has one
 * Every entity in that container is automatically placed into the parent container of E
 * They also inherit E's position, assuming they have one.
 * @param E The entity that will lose its container
 */
extern void removeContainer(ENTITY* E)
{
  _CACHE_OF(ENTITY)* container;
  GetDataFlag(E, FLAG_CONTAINER, (void**)&container);
  if (!container)
    return;
  uint containerLength = cacheLength(_CACHE(*container));
  ENTITY* CONTAINING_ENT;
  GetDataFlag(E, FLAG_CONTAINEDBY, (void**)&CONTAINING_ENT);
  int EZ, EX, EY;
  int *Pos;
  GetDataFlag(E, FLAG_POS, (void**)&Pos);
  if (Pos)
  {
    ConvertToZXY(*Pos, &EZ, &EX, &EY);
  }
  int i;
  for (i = 0; i < containerLength; i++)
  {
    ENTITY* access = cacheAccess(_CACHE(*container), i);
    if (!access) // This is probably impossible.
      continue;
    ClearDataFlag(access, FLAG_CONTAINEDBY);
    if (CONTAINING_ENT)
      addEntToContainer(CONTAINING_ENT, access);
    if (Pos && HasDataFlag(access, FLAG_POS))
    {
      uint *AccessPos;
      GetDataFlag(access, FLAG_POS, (void**)&AccessPos);
      ConvertToPosDat(EZ, EX, EY, AccessPos);
    }
  }
  cacheDelete(_CACHE(*container));
  ClearDataFlag(E, FLAG_CONTAINER);
}

/**
 * Sets color info for an entity.
 * Passing -1 to any of the color parameters keeps the current value.
 * @param In The entity
 * @param Color The primary color
 * @param Backcolor The secondary color
 * @param RenderOrder The render order
 */
extern void entSetColor(ENTITY* In, int Color, int Backcolor, int RenderOrder)
{
  B_PIXEL* Current;
  GetDataFlag(In, FLAG_APPEARANCE, (void**)&Current);
  Color = Color != -1 ? Color : Current->color;
  Backcolor = Backcolor != -1 ? Backcolor : Current->backcolor;
  RenderOrder = RenderOrder != -1 ? RenderOrder : Current->renderorder;
  B_PIXEL* New = malloc(sizeof(B_PIXEL));
  New->text = Current->text;
  New->color = Color;
  New->backcolor = Backcolor;
  New->renderorder = RenderOrder;
  SetDataFlag(In, FLAG_APPEARANCE, New);
}

/**
 * Sets the name of an entity.
 */
extern void entSetName(ENTITY* In, char *newName)
{
  int len = strlen(newName);
  char *nameSafe = calloc(len + 1, sizeof(char));
  int i;
  for (i = 0; i < len; i++)
  {
    nameSafe[i] = newName[i];
  }
  SetDataFlag(In, FLAG_NAME, nameSafe);
}

/**
 * Generates an entity with most required parameters
 * @param In The entity it should be spawned inside of
 * @param X X position
 * @param Y Y position
 * @param Z Z rendering order
 * @param Text The text of the entity
 * @return The entity
 */
extern ENTITY* entFactory(ENTITY* In, const int X, const int Y, const int Z, const char Text)
{
  ENTITY* E;
  CreateEntity(&E);
  B_PIXEL* EP = malloc(sizeof(EP));
  EP->text = Text;
  EP->color = B_DEFAULT_COLOR;
  EP->backcolor = 0;
  EP->renderorder = 0;
  SetDataFlag(E, FLAG_APPEARANCE, EP);
  SetEntPos(E, X, Y, Z);
  if (In)
    addEntToContainer(In, E);

  random_scramble(0x592da9b0f92c2d02 ^ (long)&E);

  return E;
}

/**
 * Generates an entity template.
 */
extern ENTITY* entPrefab(const enum ENT_PREFAB PrefabID, ENTITY* In, const int X, const int Y, const int Z)
{
  switch (PrefabID)
  {
  default: break;
  case PREFAB_WALL:
    {
      ENTITY* ent = entFactory(In, X, Y, Z, '#');
      entSetColor(ent, 240, 0, 127);
      entSetName(ent, "Wall");
      SetDataFlag(ent, FLAG_HEALTH, int2ap(25, 25));
      SetDataFlag(ent, FLAG_ARMOR, intap(5));
      SetBoolFlag(ent, BFLAG_DESTRUCTIBLE);
      SetBoolFlag(ent, BFLAG_OCCLUDING);
      SetBoolFlag(ent, BFLAG_STOPPING);
      SetBoolFlag(ent, BFLAG_STATIC);
      return ent;
    }
  case PREFAB_WINDOW:
    {
      ENTITY* ent = entFactory(In, X, Y, Z, '[');
      entSetColor(ent, 75, 0, 63);
      entSetName(ent, "Window");
      SetDataFlag(ent, FLAG_HEALTH, int2ap(10, 10));
      SetDataFlag(ent, FLAG_ARMOR, intap(1));
      SetBoolFlag(ent, BFLAG_DESTRUCTIBLE);
      SetBoolFlag(ent, BFLAG_STOPPING);
      SetBoolFlag(ent, BFLAG_STATIC);
      return ent;
    }
  case PREFAB_PUSHBOX:
    {
      ENTITY* ent = entFactory(In, X, Y, Z, 'M');
      entSetColor(ent, 202, 0, 127);
      entSetName(ent, "Box");
      SetDataFlag(ent, FLAG_HEALTH, int2ap(5, 5));
      SetBoolFlag(ent, BFLAG_DESTRUCTIBLE);
      SetBoolFlag(ent, BFLAG_PUSHABLE);
      return ent;
    }
  case PREFAB_FRAGILEWALL:
    {
      ENTITY* ent = entFactory(In, X, Y, Z, '#');
      entSetColor(ent, 81, 0, 127);
      entSetName(ent, "Weak wall");
      SetDataFlag(ent, FLAG_HEALTH, int2ap(15, 15));
      SetDataFlag(ent, FLAG_FRAGILE, intap(15));
      SetBoolFlag(ent, BFLAG_DESTRUCTIBLE);
      SetBoolFlag(ent, BFLAG_OCCLUDING);
      SetBoolFlag(ent, BFLAG_STOPPING);
      SetBoolFlag(ent, BFLAG_STATIC);
      return ent;
    }
  }
  return entFactory(In, X, Y, Z, 'X');
}

/**
 * Generates a game
 * @param out The return value
 */
extern void generateGame(ENTITY** out)
{
  ENTITY* game;
  CreateEntity(&game);
  ENTITY* map;
  CreateEntity(&map);
  SetBoolFlag(map, BFLAG_ISMAP);
  addContainer(map);
  _CACHE_OF(ENTITY) mapCache;
  cacheInit(_CACHE_REF(&mapCache));
  cacheAdd(_CACHE_REF(&mapCache), map);
  SetDataFlag(game, FLAG_MAPS, mapCache);
  ENTITY* gravity;
  CreateEntity(&gravity);
  AddController(gravity, CreateController(CONT_ZVEL_HANDLE));
  int x;
  for (x = 0; x < MAP_WIDTH; x++)
  {
    if (x > 12)
    {
      entPrefab(PREFAB_WINDOW, map, x, 2, x - 12);
      entPrefab(PREFAB_WINDOW, map, x, 3, x - 12);
    }
    int y;
    for (y = 0; y < MAP_HEIGHT; y++)
    {
      int targetPrefab = (y == 20 || y == 21 || y == 22) ? PREFAB_WINDOW : PREFAB_WALL;
      entPrefab(PREFAB_WALL, map, x, y, 0);
      if (x >= 20 && x < 30 && y == 5)
      {
        int z = x - 19 + 4;
        ENTITY *numbertile = entFactory(map, x, y, z, '0' + x - 20);
        char naem[2];
        naem[0] = '0' + x - 20;
        naem[1] = 0;
        entSetName(numbertile, naem);
        SetBoolFlag(numbertile, BFLAG_STATIC);
      }
      if (x >= 16)
      {
        int shed = (x == 40 || x == 41) ? 3 : 4;
        for (; shed >= 3; shed--)
          entPrefab(targetPrefab, map, x, y, shed);
      }
      int pX = 41 - x;
      int pY = 40 - y;
      if (pX < 0) pX = -pX;
      if (pY < 0) pY = -pY;
      int mi = max(pX, pY);
      int pZ = 15 - mi;
      int i;
      for (i = pZ * 2 - ((x + y) % 2); i > 4; i--)
      {
        entSetName(entPrefab(PREFAB_WALL, map, x, y, i), "Pyramid");
      }
    }
  }
  entPrefab(PREFAB_FRAGILEWALL, map, 9, 9, 1);

  ENTITY* playerEnt = entFactory(map, 1, 1, 1, '@');
  entSetColor(playerEnt, 0, 12, 255);
  char *playeynamey;

  entSetName(playerEnt, "Player");

  SetDataFlag(playerEnt, FLAG_HEALTH, int2ap(100, 100));
  SetDataFlag(playerEnt, FLAG_SPEED, intap(50));
  SetDataFlag(playerEnt, FLAG_CANJUMP, intap(4));
  SetDataFlag(playerEnt, FLAG_SIGHTRANGE, intap(20));

  SetBoolFlag(playerEnt, BFLAG_CLIMBER);

  SetDataFlag(game, FLAG_PLAYER, playerEnt);

  B_BUFFER* buffer;
  b_initialize(&buffer);
  SetDataFlag(game, FLAG_APPEARANCE, buffer);
  *out = game;
}
