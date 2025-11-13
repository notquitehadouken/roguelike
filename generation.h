#pragma once
#include "global.h"
#include "object.h"

/**
 * Adds an entity to a container
 * @param container The container
 * @param ent The entity
 */
extern void addEntToContainer(ENTITY* container, ENTITY* ent)
{
  ENTITY** ELIST; // (-1) [cursed]
  GetDataFlag(container, FLAG_CONTAINER, (void**)&ELIST);
  for (int i = 0; i < CONTAINERCAPACITY /* If it hits CONTAINERCAPACITY entities,
        you are fucked.*/; i++)
  {
    if (!ELIST[i])
    {
      ENTITY** containerRef = malloc(sizeof(ENTITY*));
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
extern void removeEntFromContainer(const ENTITY* container, ENTITY* ent)
{
  ENTITY** ELIST; // (-1) [cursed]
  GetDataFlag(container, FLAG_CONTAINER, (void**)&ELIST);
  for (int i = 0; i < CONTAINERCAPACITY; i++)
  {
    if (ELIST[i] == ent)
    {
      ClearDataFlag(ent, FLAG_CONTAINEDBY);
      ELIST[i] = nullptr;
      return;
    }
  }
}

/**
 * Adds a container to an entity if one does not exist already
 * @param E The entity
 */
extern void addContainer(ENTITY* E)
{
  if (HasDataFlag(E, FLAG_CONTAINER))
    return;
  void** LIST = malloc(CONTAINERCAPACITY * sizeof(int*));
  for (int i = 0; i < CONTAINERCAPACITY; i++)
    LIST[i] = nullptr;
  SetDataFlag(E, FLAG_CONTAINER, LIST);
}

/**
 * Removes an entity's container if it has one
 * Every entity in that container is automatically placed into the parent container of E
 * They also inherit E's position, assuming they have one.
 * @param E The entity that will lose its container
 */
extern void removeContainer(ENTITY* E)
{
  ENTITY** ELIST;
  GetDataFlag(E, FLAG_CONTAINER, (void**)&ELIST);
  if (!ELIST)
    return;
  ClearDataFlag(E, FLAG_CONTAINER); // Time to rescue all the entities from the void.
  ENTITY* CONTAINING_ENT;
  GetDataFlag(E, FLAG_CONTAINEDBY, (void**)&CONTAINING_ENT);
  for (int i = 0; i < CONTAINERCAPACITY; i++)
  {
    ENTITY* E2 = ELIST[i];
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
extern ENTITY* entFactory(ENTITY* In, const int X, const int Y, const unsigned char Z, const char Text,
                          const unsigned char Color)
{
  ENTITY* E;
  CreateEntity(&E);
  B_PIXEL* EP = malloc(sizeof(EP));
  EP->text = Text;
  EP->color = Color;
  EP->backcolor = 0;
  EP->effect = 0;
  SetDataFlag(E, FLAG_APPEARANCE, EP);
  unsigned int* P = malloc(sizeof(unsigned int));
  ConvertToPosDat(Z, X, Y, P);
  SetDataFlag(E, FLAG_POS, P);
  if (In)
    addEntToContainer(In, E);

  random_scramble(0x592da9b0f92c2d02 ^ (long)&E);

  return E;
}

/**
 * Sets color info for an entity.
 * Passing -1 to any of the color parameters keeps the current value.
 * @param In The entity
 * @param Color The primary color
 * @param Backcolor The secondary color
 * @param Effect The effect
 */
extern void entSetColor(ENTITY* In, int Color, int Backcolor, int Effect)
{
  B_PIXEL *Current;
  GetDataFlag(In, FLAG_APPEARANCE, (void**)&Current);
  Color = Color != -1 ? Color : Current->color;
  Backcolor = Backcolor != -1 ? Backcolor : Current->backcolor;
  Effect = Effect != -1 ? Effect : Current->effect;
  B_PIXEL *New = malloc(sizeof(B_PIXEL));
  New->text = Current->text;
  New->color = Color;
  New->backcolor = Backcolor;
  New->effect = Effect;
  SetDataFlag(In, FLAG_APPEARANCE, New);
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
  ENTITY** maps = malloc(GAMETOTALMAPS * sizeof(map));
  maps[0] = map;
  SetDataFlag(game, FLAG_MAPS, maps);
  for (int x = 0; x < MAP_WIDTH; x++)
    for (int y = 0; y < MAP_HEIGHT; y++)
    {
      entFactory(map, x, y, 0, '.', 59);
      if (y >= 2 && x < 16)
      {
        const int cocaine = (y - 2) * 16 + x;
        if (cocaine < 512)
        {
          ENTITY *test = entFactory(map, x * 2, y, 1, cocaine / 2, 15);
          if (cocaine % 2)
            entSetColor(test, 0, 4, 5);
        }
      }
      if (x >= 14 && y >= 28)
      {
        if (x % 7 == 0 && y % 7 == 0)
        {
          ENTITY* wall = entFactory(map, x, y, 127, '#', 188);
          SetBoolFlag(wall, BFLAG_COLLIDABLE);
          SetBoolFlag(wall, BFLAG_OCCLUDING);
        }
        else if ((x % 7 == 0 || y % 7 == 0) && random_nextInt() % 2 == 0)
        {
          ENTITY* wall = entFactory(map, x, y, 127, '#', 102);
          SetBoolFlag(wall, BFLAG_COLLIDABLE);
          SetBoolFlag(wall, BFLAG_OCCLUDING);
        }
      }
      if (x >= 20 && y >= 40 && random_nextInt() % 30 == 1)
      {
        ENTITY* follower = entFactory(map, x, y, 255, '@', 33);
        SetBoolFlag(follower, BFLAG_COLLIDABLE);
        ENTITY_CONTROLLER* Controller = CreateController(CONT_MOVETOPLAYER);
        Controller->nextAct = GLOBAL_TIMER;
        AddController(follower, Controller);
        addEntToContainer(map, follower);
        int* S = malloc(sizeof(S));
        *S = 100;
        SetDataFlag(follower, FLAG_SPEED, S);
      }
      if (y > 35)
      {
        if (x >= 25 && !(x % 9))
        {
          ENTITY* floor = entFactory(map, x, y, 1, '*', 36);
          int* SC = malloc(sizeof(int));
          *SC = -3;
          SetDataFlag(floor, FLAG_CHANGE_SPEED_ON_STEP, SC);
        }
        if (x == 10)
        {
          ENTITY* floor = entFactory(map, x, y, 1, '*', 31);
          int* SC = malloc(sizeof(int));
          *SC = 3;
          SetDataFlag(floor, FLAG_CHANGE_SPEED_ON_STEP, SC);
        }
      }
    }

  ENTITY* playerEnt = entFactory(map, 0, 0, 255, '@', 31);
  entSetColor(playerEnt, -1, -1, 7);
  SetDataFlag(playerEnt, FLAG_NAME, "Player");

  int* HP = malloc(2 * sizeof(int));
  HP[0] = 100;
  HP[1] = 100;
  SetDataFlag(playerEnt, FLAG_HEALTH, HP);

  int* SP = malloc(sizeof(int));
  *SP = 5;
  SetDataFlag(playerEnt, FLAG_SPEED, SP);

  int* VR = malloc(sizeof(int));
  *VR = 20;
  SetDataFlag(playerEnt, FLAG_SIGHTRANGE, VR);

  SetDataFlag(game, FLAG_PLAYER, playerEnt);


  B_BUFFER* buffer;
  b_initialize(&buffer);
  SetDataFlag(game, FLAG_APPEARANCE, buffer);
  *out = game;
}
