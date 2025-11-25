/// <summary>
///     This header contains TIMEOF data, which is the amount of time that certain actions take to complete
///     And also contains all the unwieldy functions that convert between certain types
///     And handles all entity-entity behavior
///     It is distinct from global.h in that it requires all other header files in order to function properly
///     As a consequence, no header files use acts.h
/// </summary>
#pragma once
#include "global.h"
#include "keyhandler.h"
#include "object.h"
#include "screen.h"

/**
 * Disconnects an entity from the rest of the world, and deletes it.
 * Dust to dust, mama-fucker!
 */
extern void BreakEntity(ENTITY *E)
{
  if (HasDataFlag(E, FLAG_POS))
    MoveEnt(E, -1, -1); // Out my position buffer.
  if (HasDataFlag(E, FLAG_CONTAINEDBY))
  {
    ENTITY** Container;
    GetDataFlag(E, FLAG_CONTAINEDBY, (void**)&Container);
    removeEntFromContainer(*Container, E);
  }
  if (HasDataFlag(E, FLAG_CONTAINER))
  {
    removeContainer(E);
  }
  DeleteEntity(E);
}

/**
 * Converts a direction constant to X and Y values
 * Up is considered (0, -1)
 * @param dir The direction
 * @param outX The X as output
 * @param outY The Y as output
 */
extern void DirToVec2(const char dir, int* outX, int* outY)
{
  switch (dir)
  {
  case UP_KEEP:
  case UP:
    {
      *outY = -1;
      *outX = 0;
      break;
    }
  case DOWN_KEEP:
  case DOWN:
    {
      *outY = 1;
      *outX = 0;
      break;
    }
  case LEFT_KEEP:
  case LEFT:
    {
      *outY = 0;
      *outX = -1;
      break;
    }
  case RIGHT_KEEP:
  case RIGHT:
    {
      *outY = 0;
      *outX = 1;
      break;
    }
  case UPLEFT_KEEP:
  case UPLEFT:
    {
      *outY = -1;
      *outX = -1;
      break;
    }
  case UPRIGHT_KEEP:
  case UPRIGHT:
    {
      *outY = -1;
      *outX = 1;
      break;
    }
  case DOWNLEFT_KEEP:
  case DOWNLEFT:
    {
      *outY = 1;
      *outX = -1;
      break;
    }
  case DOWNRIGHT_KEEP:
  case DOWNRIGHT:
    {
      *outY = 1;
      *outX = 1;
      break;
    }
  default:
    {
      *outY = 0;
      *outX = 0;
      break;
    }
  }
}

extern void DirToVec3(const char dir, int* outX, int* outY, int* outZ)
{
  *outX = 0;
  *outY = 0;
  *outZ = 0;
  switch (dir)
  {
  case HEIGHTUP:
  case HEIGHTUP_KEEP:
  case KEY_VIEWHEIGHT_C_RAISE:
    {
      *outZ = 1;
      break;
    }
  case HEIGHTDOWN:
  case HEIGHTDOWN_KEEP:
  case KEY_VIEWHEIGHT_C_LOWER:
    {
      *outZ = -1;
      break;
    }
  default:
    DirToVec2(dir, outX, outY);
  }
}

/**
 * Converts X and Y to a direction constant
 * @param x The X
 * @param y The Y
 * @param outD The direction constant as output
 */
extern void Vec2ToDir(const int x, const int y, char* outD)
{
  if (x == 0 && y == 0)
  {
    *outD = 0;
    return;
  }
  if (x == 0 && y > 0)
  {
    *outD = DOWN;
    return;
  }
  if (x == 0 && y < 0)
  {
    *outD = UP;
    return;
  }

  if (x < 0 && y == 0)
  {
    *outD = LEFT;
    return;
  }
  if (x < 0 && y > 0)
  {
    *outD = DOWNLEFT;
    return;
  }
  if (x < 0 && y < 0)
  {
    *outD = UPLEFT;
    return;
  }

  if (x > 0 && y == 0)
  {
    *outD = RIGHT;
    return;
  }
  if (x > 0 && y > 0)
  {
    *outD = DOWNRIGHT;
    return;
  }
  if (x > 0 && y < 0)
  {
    *outD = UPRIGHT;
    return;
  }
  *outD = 0;
}

extern void Vec3ToDir(const int x, const int y, const int z, char* outD)
{
  if (z == 0)
  {
    Vec2ToDir(x, y, outD);
    return;
  }
  *outD = (z > 0) ? HEIGHTUP : HEIGHTDOWN;
}

/**
 * Selects a position, starting from the passed values.
 */
extern void PositionSelect(B_BUFFER *buffer, const ENTITY *map, const ENTITY *player, int *Xp, int *Yp, int *Zp)
{
  int X = *Xp, Y = *Yp, Z = *Zp;
  int oldVH = mapViewHeight;
  int oldVHC = mapCustomVH;
  mapViewHeight = VIEWHEIGHT_CUSTOM;
  mapCustomVH = Z;

  for (;;)
  {
    b_writeMapToBuffer(buffer, map, player);
    b_setEffect(buffer, Y + 2, X, 7);
    b_clearRow(buffer, 0);
    const ENTITY *Target = entitiesDrawn[X + Y * MAP_WIDTH];
    if (Target)
    {
      if (HasDataFlag(Target, FLAG_NAME))
      {
        char *Name;
        GetDataFlag(Target, FLAG_NAME, (void**)&Name);
        b_writeTo(buffer, 0, 0, Name);
      }
      else
      {
        b_writeTo(buffer, 0, 0, "No name");
      }
    }
    else
    {
      b_writeTo(buffer, 0, 0, "I can't see there.");
    }
    b_draw(buffer);
    int nextMove = getNextInput();
    if (!inRange(nextMove, 0, 127))
      nextMove = 0;
    if (nextMove == CONFIRM)
      break;
    int dX, dY, dZ;
    DirToVec3(nextMove, &dX, &dY, &dZ);
    if (InBounds3D(X + dX, Y + dY, Z + dZ))
    {
      X += dX;
      Y += dY;
      Z += dZ;
    }
    mapCustomVH = Z;
  }

  *Xp = X;
  *Yp = Y;
  *Zp = Z;

  mapCustomVH = oldVHC;
  mapViewHeight = oldVH;
}

/**
 * Changes the health of an entity directly
 * Positive means healing
 * Only changes health
 * @param E The entity
 * @param Change The change in health
 */
extern void ChangeHealth(const ENTITY* E, const int Change)
{
  int* HP;
  GetDataFlag(E, FLAG_HEALTH, (void**)&HP);
  if (!HP)
    return;
  HP[0] += Change;
  if (HP[0] > HP[1])
    HP[0] = HP[1];
  if (HP[0] < 0)
    HP[0] = 0;
}

/**
 * Damages an entity.
 * This may destroy an entity.
 * @returns Leftover damage.
 */
extern uint TryDamage(ENTITY* E, uint Damage)
{
  if (Damage == 0)
    return 0;
  int* HP = 0;
  GetDataFlag(E, FLAG_HEALTH, (void**)&HP);
  if (!HP)
    return Damage;
  int* Armor = 0;
  GetDataFlag(E, FLAG_ARMOR, (void**)&Armor);
  if (Damage > 0)
  {
    Damage = max(0, Damage - (Armor ? *Armor : 0));
  }
  uint Remaining = 0;
  if (*HP - Damage < 0)
  {
    Remaining = Damage - *HP;
  }
  ChangeHealth(E, -Damage);
  if (*HP <= 0)
  {
    if (HasBoolFlag(E, BFLAG_DESTRUCTIBLE))
      BreakEntity(E);
    return Remaining;
  }
  return 0;
}

extern int TryMove(ENTITY*, int); // Shut up, Collide.

/**
 * The function called when something moves into something else
 * If Mover is 0, this function is just if MovedInto would be collided with normally
 * @param Mover The moving entity
 * @param MovedInto The entity moved into
 * @param dir The direction of movement
 * @return If the movement was blocked
 */
extern char Collide(const ENTITY* Mover, ENTITY* MovedInto, const int dir)
{
  if (!Mover)
  {
    if (HasBoolFlag(MovedInto, BFLAG_PUSHABLE)
      || HasBoolFlag(MovedInto, BFLAG_STOPPING))
      return 1;
    return 0;
  }
  if (HasDataFlag(MovedInto, FLAG_FRAGILE) && dir != 0)
  {
    int *DamageTaken;
    GetDataFlag(MovedInto, FLAG_FRAGILE, (void**)&DamageTaken);
    TryDamage(MovedInto, *DamageTaken);
  }
  if (HasBoolFlag(MovedInto, BFLAG_PUSHABLE))
  {
    return TryMove(MovedInto, dir) == -1;
  }
  if (HasBoolFlag(MovedInto, BFLAG_STOPPING))
  {
    return 1;
  }
  return 0;
}

/**
 * Only supports a unit dZ.
 */
extern int TryMoveVertical(ENTITY* E, const int dZ)
{
  if (dZ == 0)
    return -1;
  int z, x, y;
  uint* posDat;
  ENTITY** mapRef;
  GetDataFlag(E, FLAG_CONTAINEDBY, (void**)&mapRef);
  if (!mapRef) // Something has gone wrong, but sure.
    return -1;
  const ENTITY* map = *mapRef;
  GetDataFlag(E, FLAG_POS, (void**)&posDat);
  ConvertToZXY(*posDat, &z, &x, &y);
  const int targetZ = z + dZ;
  if (!inRange(targetZ, 0, MAP_DEPTH - 1))
    return -1;
  ENTITY** ELIST;
  int count = 0;
  if (!GetEntitiesOnPosition(map, x, y, &ELIST, &count))
  {
    free(ELIST);
    return -1; // Failure of this method means the map does not exist.
  }
  char *EntitiesHit = calloc(count, sizeof(char));
  int LevitationPos = 0;
  if (HasDataFlag(E, FLAG_LEVITATE))
  {
    int *L;
    GetDataFlag(E, FLAG_LEVITATE, (void**)&L);
    LevitationPos = *L;
  }
  int i;
  char Supported = 0;
  char LowerSpot = 1;
  if (z <= LevitationPos || dZ < 0)
  {
    Supported = 1;
    LevitationPos = 0;
  }
  for (i = 0; i <= count; i++)
  {
    if (i == count - 1 && (LowerSpot < LevitationPos && !Supported))
    {
      i = -1;
      LowerSpot += 1;
      continue;
    }
    if (i == count)
      break;
    if (EntitiesHit[i])
      continue;
    ENTITY* AeE = ELIST[i];
    if (GetEntZ(AeE) == targetZ)
    {
      EntitiesHit[i] = 1;
      if (Collide(E, AeE, 0))
      {
        free(ELIST);
        free(EntitiesHit);
        return -1;
      }
    }
    if (LevitationPos == 0)
      continue;
    if (GetEntZ(AeE) == z - LowerSpot && Collide(E, AeE, 0))
    {
      EntitiesHit[i] = 1;
      Supported = 1;
      LevitationPos = 0;
    }
  }
  free(EntitiesHit);
  free(ELIST);
  if (!Supported)
    return -1;
  SetEntZ(E, targetZ);
  return ScaleTime(TIMEOF_MOVE, E);
}

/**
 * Attempt to move an entity with this direction constant
 * @param E The entity to be moved
 * @param dir The direction constant
 * @return How long movement took. -1 means the movement failed, because 0-time movement is possible.
 */
extern int TryMove(ENTITY* E, const int dir)
{
  int desiredDX, desiredDY;
  DirToVec2(dir, &desiredDX, &desiredDY);
  if (desiredDX == 0 && desiredDY == 0)
    return -1;
  int z, curX, curY;
  uint* posDat;
  ENTITY** mapRef;
  GetDataFlag(E, FLAG_CONTAINEDBY, (void**)&mapRef);
  if (!mapRef) // Something has gone wrong, but sure.
    return -1;
  const ENTITY* map = *mapRef;
  GetDataFlag(E, FLAG_POS, (void**)&posDat);
  ConvertToZXY(*posDat, &z, &curX, &curY);
  const int x = curX + desiredDX;
  const int y = curY + desiredDY;
  int targetZ = z;
  if (!InBounds(x, y))
    return -1;
  ENTITY** ELIST;
  int count = 0;
  if (!GetEntitiesOnPosition(map, x, y, &ELIST, &count))
  {
    free(ELIST);
    return -1; // Failure of this method means the map does not exist.
  }
  int i;
  const char canClimb = HasBoolFlag(E, BFLAG_CLIMBER);
  char frontBlocked = 0;
  char topBlocked = 0;
  char aboveBlocked = 0;
  if (canClimb)
  {
    ENTITY** ELISTSAMEPOS;
    int countSamePos = 0;
    if (!GetEntitiesOnPosition(map, curX, curY, &ELISTSAMEPOS, &countSamePos))
    {
      free(ELIST);
      free(ELISTSAMEPOS);
      return -1; // Failure of this method means the map does not exist.
    }
    for (i = 0; i < countSamePos; i++)
    {
      int Z, u;
      int* EPos;
      GetDataFlag(ELISTSAMEPOS[i], FLAG_POS, (void**)&EPos);
      ConvertToZXY(*EPos, &Z, &u, &u);
      if (Z == z + 1 && Collide(E, ELISTSAMEPOS[i], dir))
      {
        aboveBlocked = 1;
        break;
      }
    }
    free(ELISTSAMEPOS);
  }
  for (i = 0; i < count; i++)
  {
    int Z, u;
    int* EPos;
    GetDataFlag(ELIST[i], FLAG_POS, (void**)&EPos);
    ConvertToZXY(*EPos, &Z, &u, &u);
    if (Z == z && !frontBlocked)
    {
      if (Collide(E, ELIST[i], dir))
        frontBlocked = 1;
      continue;
    }
    if (canClimb && Z == z + 1 && !topBlocked)
    {
      if (Collide(E, ELIST[i], dir))
        topBlocked = 1;
    }
  }
  if (frontBlocked)
  {
    if (!canClimb || topBlocked || aboveBlocked)
    {
      return -1;
    }
    targetZ++;
  }
  free(ELIST);
  MoveEnt(E, x, y);
  SetEntZ(E, targetZ);
  return ScaleTime(TIMEOF_MOVE, E);
}

/**
 * Shoot a damaging ray in a direction.
 * It will attempt to expend all its damage.
 * (SX, SY, SZ) and (EX, EY, EZ) are clamped to the map
 */
extern void DamageRay(const ENTITY* map, int Damage, int SX, int SY, int SZ, int EX, int EY, int EZ)
{
  if (Damage == 0)
    return;
  SX = clamp(SX, 0, MAP_WIDTH - 1);
  SY = clamp(SY, 0, MAP_HEIGHT - 1);
  SZ = clamp(SZ, 0, MAP_DEPTH - 1);
  EX = clamp(EX, 0, MAP_WIDTH - 1);
  EY = clamp(EY, 0, MAP_HEIGHT - 1);
  EZ = clamp(EZ, 0, MAP_DEPTH - 1);
  int DX = EX - SX;
  int DY = EY - SY;
  int DZ = EZ - SZ;
  int count;
  int **Line = createLine(DX, DY, DZ, 0, &count);
  int i = 0;
  int ECount;
  ENTITY **Array = 0;
  char RefreshArray = 0;
  for (i = 0; i < count; i++)
  {
    if (Damage == 0)
      break;
    if (i == 0)
    {
      GetEntitiesOnPosition(map, Line[i][0] + SX, Line[i][1] + SY, &Array, &ECount);
    }
    else if (RefreshArray || Line[i][0] != Line[i-1][0] || Line[i][1] != Line[i-1][1])
    {
      free(Array);
      GetEntitiesOnPosition(map, Line[i][0] + SX, Line[i][1] + SY, &Array, &ECount);
      RefreshArray = 0;
    }
    int j;
    for (j = 0; j < ECount; j++)
    {
      if (Damage == 0)
        break;
      if (GetEntZ(Array[j]) != Line[i][2] + SZ)
        continue;
      RefreshArray = 1;
      Damage = TryDamage(Array[j], Damage);
    }
  }
  if (Array)
    free(Array);
  freeLine(Line, count);
}

/**
 * I don't CARE how big the room is, I CAST FIREBALL!
 */
extern void Explode(const ENTITY* map, const int Range, const int Power, const int X, const int Y, const int Z)
{
  if (Power == 0)
    return;
  int TX, TY, TZ;
  for (TX = Range; TX <= Range; flipper(&TX))
  {
    for (TY = 0; TY <= Range; flipper(&TY))
    {
      for (TZ = 0; TZ < Range; flipper(&TZ))
      {
        DamageRay(map, Power, X, Y, Z, X + TX, Y + TY, Z + TZ);
        DamageRay(map, Power, X, Y, Z, X + TZ, Y + TX, Z + TY);
        DamageRay(map, Power, X, Y, Z, X + TY, Y + TZ, Z + TX);
      }
    }
  }
  for (TX = Range; TX <= Range; flipper(&TX))
  {
    for (TY = Range; TY <= Range; flipper(&TY))
    {
      for (TZ = Range; TZ <= Range; flipper(&TZ))
      {
        DamageRay(map, Power, X, Y, Z, X + TX, Y + TY, Z + TZ);
      }
    }
  }
}

/**
 * Handles controller logic. Called every game loop for every entity with a controller.
 * @param controller The controller object
 * @param game The game object
 * @return If anything happened
 */
extern char ControllerProcess(ENTITY_CONTROLLER* controller, const ENTITY* game)
{
  switch (controller->type)
  {
  default:
  case CONT_NOACTION:
  case CONT_DEAD:
    {
      controller->nextAct = GLOBAL_TIMER + 1;
      return 0;
    }
  case CONT_ZVEL_HANDLE:
    {
      static const int timePerAccel = 50;
      static const int granularity = 5; // must be divisible by timePerAccel
      static const int stepSize = timePerAccel / granularity;
      const int stepID = (controller->nextAct % timePerAccel) / stepSize;
      controller->nextAct += stepSize;
      int i;
      int *StartIndForZ = calloc(MAP_DEPTH, sizeof(int));
      int *EndIndForZ = calloc(MAP_DEPTH, sizeof(int));
      for (i = 0; i < MAP_CELLCOUNT; i++)
      {
        const int X = i % MAP_WIDTH;
        const int Y = i / MAP_WIDTH;
        ENTITY** ELIST = GetEntsAtPos(X, Y);
        int len = 0;
        for (; ELIST[len]; len++);
        unsigned char* EPOS = calloc(len, sizeof(char));
        ENTITY** MapOf = calloc(len, sizeof(ENTITY*));

        int j;

        for (j = 0; j < MAP_DEPTH; j++)
        {
          StartIndForZ[j] = -1;
          EndIndForZ[j] = -1;
        }

        for (j = 0; j < len; j++)
        {
          const ENTITY* E = ELIST[j];
          const int Z = GetEntZ(E);
          if (Z < 0)
            continue;
          EPOS[j] = Z;
          ENTITY** Container;
          GetDataFlag(E, FLAG_CONTAINEDBY, (void**)&Container);
          MapOf[j] = *Container;
          EndIndForZ[Z] = j;
          if (StartIndForZ[Z] == -1)
            StartIndForZ[Z] = j;
        }

        int Z, cI;

        for (Z = 0; Z < MAP_DEPTH; Z++) // Shit on Z 0 can't fall.
        {
          if (StartIndForZ[Z] == -1)
            continue;
          for (cI = StartIndForZ[Z]; cI <= EndIndForZ[Z]; cI++)
          {
            if (EPOS[cI] != Z)
              continue;
            ENTITY* FallingEntity = ELIST[cI];
            if (!MapOf[cI])
              continue;
            if (HasBoolFlag(FallingEntity, BFLAG_STATIC))
              continue;

            if (!HasDataFlag(FallingEntity, FLAG_ZVEL))
              SetDataFlag(FallingEntity, FLAG_ZVEL, intap(0));
            int* ZVel;

            GetDataFlag(FallingEntity, FLAG_ZVEL, (void**)&ZVel);
            if (stepID == granularity - 1)
              *ZVel += 1;

            const char ZVelS = sign(*ZVel);

            if (ZVelS == 0)
              continue;

            const int ZVelUsed = ZVelS * ((ZVelS * *ZVel + stepID) / granularity);

            if (ZVelUsed == 0)
              continue;

            const int ZVelSign = sign(ZVelUsed); // < 0 means upward.

            int LevitateHeight = 0;
            if (ZVelSign > 0 && HasDataFlag(FallingEntity, FLAG_LEVITATE))
            {
              int* H;
              GetDataFlag(FallingEntity, FLAG_LEVITATE, (void**)&H);
              LevitateHeight = *H;
              if (LevitateHeight > MAP_DEPTH)
              {
                *ZVel = 0;
                continue; // Flight assumed.
              }
            }
            const int ZNextTo = Z - ZVelSign;
            int ZTarget = clamp(Z - ZVelUsed - LevitateHeight, 0, MAP_DEPTH - 1) + LevitateHeight;
            int ZI;
            char CollisionFound = 0;
            for (ZI = ZNextTo; ZI != ZTarget - LevitateHeight - ZVelSign; ZI -= ZVelSign)
            {
              if (StartIndForZ[ZI] == -1)
                continue;
              int cI2;
              for (cI2 = StartIndForZ[ZI]; cI2 <= EndIndForZ[ZI]; cI2++)
              {
                if (EPOS[cI2] != ZI)
                  continue;
                if (MapOf[cI] != MapOf[cI2])
                  continue;
                ENTITY* EntityBelow = ELIST[cI2];
                if (Collide(FallingEntity, EntityBelow, 0))
                {
                  if (ZVelSign > 0)
                  {
                    ZTarget = min(Z, ZI + 1 + LevitateHeight);
                  }
                  else
                  {
                    ZTarget = max(Z, ZI - 1);
                  }
                  *ZVel = 0;
                  CollisionFound = 1;
                  break;
                }
              }
              if (CollisionFound)
                break;
            }
            if (ZVelSign > 0 && ZTarget <= 0)
              *ZVel = 0;
            if (ZVelSign < 0 && ZTarget >= MAP_DEPTH - 1)
              *ZVel = 0;
            SetEntZ(FallingEntity, ZTarget);
          }
        }

        free(MapOf);
        free(EPOS);
        free(ELIST);
      }
      free(StartIndForZ);
      free(EndIndForZ);
      return 1;
    }
  case CONT_MOVETOPLAYER:
    {
      ENTITY* player;
      GetDataFlag(game, FLAG_PLAYER, (void**)&player);

      ENTITY* us = controller->associate;

      int pX, pY, uX, uY, *pP, *uP;
      GetDataFlag(player, FLAG_POS, (void**)&pP);
      GetDataFlag(us, FLAG_POS, (void**)&uP);
      if (!pP || !uP)
        break;

      ConvertToZXY(*pP, &pX, &pX, &pY);
      ConvertToZXY(*uP, &uX, &uX, &uY);

      char dir;
      Vec2ToDir(pX - uX, pY - uY, &dir);
      const int move = TryMove(us, dir);
      controller->nextAct += move;
      if (move <= 0)
        controller->nextAct += TIMEOF_LONGWAIT;
      return 1;
    }
  }
  return 0;
}
