/// <summary>
///     This header contains TIMEOF data, which is the amount of time that certain actions take to complete
///     And also contains all the unwieldy functions that convert between certain types
///     And handles all entity-entity behavior
///     It is distinct from global.h in that it requires all other header files in order to function properly
///     As a consequence, no header files use acts.h
/// </summary>
#pragma once
#include "generation.h"
#include "global.h"
#include "keyhandler.h"
#include "object.h"
#include "screen.h"

/**
 * Disconnects an entity from the rest of the world, and deletes it. \n
 * Dust to dust, mama-fucker!
 */
extern void BreakEntity(ENTITY* E)
{
  if (HasDataFlag(E, FLAG_POS))
  {
    MoveEnt(E, -1, -1); // Out my position buffer.
  }
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
extern void PositionSelect(B_BUFFER* buffer, const ENTITY* map, const ENTITY* player, int* Xp, int* Yp, int* Zp)
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
    const ENTITY* Target = entitiesDrawn[X + Y * MAP_WIDTH];
    if (Target)
    {
      if (HasDataFlag(Target, FLAG_NAME))
      {
        char* Name;
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

extern int TryMove3D(ENTITY*, int, int, int); // Shut up, Collide.
extern void Explode(ENTITY*, int, int, int, int, int);

/**
 * The function called when something moves into something else \n
 * If Mover is 0, this function is just if MovedInto would be collided with normally \n
 * This function can cause an entity to be destroyed. \n
 * @param Mover The moving entity
 * @param MovedInto The entity moved into
 * @param dX Change in X of mover
 * @param dY Change in Y of mover
 * @param dZ Change in Z of mover
 * @return If the movement was blocked
 */
extern char Collide(ENTITY* Mover, ENTITY* MovedInto, const int dX, const int dY, const int dZ)
{
  if (!Mover)
  {
    if (HasBoolFlag(MovedInto, BFLAG_PUSHABLE)
      || HasBoolFlag(MovedInto, BFLAG_STOPPING))
      return 1;
    return 0;
  }
  if (Mover->destroyed || MovedInto->destroyed)
    return 0;
  if (HasDataFlag(Mover, FLAG_GRENADEINFO))
  {
    if (HasDataFlag(MovedInto, FLAG_HEALTH) || Collide(0, MovedInto, 0, 0, 0))
    {
      int ExX, ExY, ExZ;
      uint *ExPos;
      GetDataFlag(Mover, FLAG_POS, (void**)&ExPos);
      ConvertToZXY(*ExPos, &ExX, &ExY, &ExZ);
      int Damage, Range;
      int *GrenInfo;
      GetDataFlag(Mover, FLAG_GRENADEINFO, (void**)&GrenInfo);
      Damage = GrenInfo[0];
      Range = GrenInfo[1];
      ENTITY **mapRef, *map;
      GetDataFlag(Mover, FLAG_CONTAINEDBY, (void**)&mapRef);
      map = *mapRef;
      BreakEntity(Mover);
      Explode(map, Damage, Range, ExX, ExY, ExZ);
      return 0;
    }
  }
  if (HasDataFlag(Mover, FLAG_BULLETINFO))
  {
    if (HasDataFlag(MovedInto, FLAG_HEALTH) || Collide(0, MovedInto, 0, 0, 0))
    {
      int *BulletInfo;
      GetDataFlag(Mover, FLAG_BULLETINFO, (void**)&BulletInfo);
      if (!HasDataFlag(MovedInto, FLAG_FRAGILE))
        BulletInfo[1]--;
      TryDamage(MovedInto, BulletInfo[0]);
      if (BulletInfo[1] < 0)
        BreakEntity(Mover);
      return 0;
    }
  }
  if (HasDataFlag(MovedInto, FLAG_FRAGILE) && (dX != 0 || dY != 0))
  {
    int* DamageTaken;
    GetDataFlag(MovedInto, FLAG_FRAGILE, (void**)&DamageTaken);
    TryDamage(MovedInto, *DamageTaken);
  }
  if (HasBoolFlag(MovedInto, BFLAG_PUSHABLE))
  {
    return TryMove3D(MovedInto, dX, dY, dZ) == -1;
  }
  if (HasBoolFlag(MovedInto, BFLAG_STOPPING))
  {
    return 1;
  }
  return 0;
}

extern int TryMove3D(ENTITY* E, const int dX, const int dY, int dZ)
{
  if (dX == 0 && dY == 0 && dZ == 0)
    return -1;
  int curZ, curX, curY;
  uint* posDat;
  ENTITY** mapRef;
  GetDataFlag(E, FLAG_CONTAINEDBY, (void**)&mapRef);
  if (!mapRef) // Something has gone wrong, but sure.
    return -1;
  ENTITY* map = *mapRef;
  GetDataFlag(E, FLAG_POS, (void**)&posDat);
  ConvertToZXY(*posDat, &curZ, &curX, &curY);

  if (HasDataFlag(E, FLAG_TRAIL))
    makeTrail(E);

  int z = curZ + dZ;
  const int x = curX + dX;
  const int y = curY + dY;
  if (!InBounds3D(x, y, z))
    return -1;

  if (dZ > 0 && !HasBoolFlag(E, BFLAG_STATIC))
  {
    ENTITY** ELIST;
    int count = 0;
    if (!GetEntitiesOnPosition(map, x, y, &ELIST, &count))
    {
      free(ELIST);
      return -1;
    }
    char* EntitiesHit = calloc(count, sizeof(char));
    int LevitationPos = 0;
    if (HasDataFlag(E, FLAG_LEVITATE))
    {
      int* L;
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
    for (i = 0; i <= count && !Supported; i++)
    {
      if (i == count - 1 && (LowerSpot < LevitationPos))
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
      if (LevitationPos == 0)
        continue;
      int TargetZ = GetEntZ(AeE);
      if (TargetZ == z - LowerSpot && Collide(E, AeE, 0, 0, dZ))
      {
        EntitiesHit[i] = 1;
        Supported = 1;
        LevitationPos = 0;
      }
    }
    free(EntitiesHit);
    free(ELIST);
    if (!Supported)
      dZ = 0;
  }

  if (E->destroyed)
    return -1; // lol.

  z = curZ + dZ;

  if (dX == 0 && dY == 0 && dZ == 0)
  {
    return -1;
  }

  ENTITY** ELIST;
  int count = 0;
  if (!GetEntitiesOnPosition(map, x, y, &ELIST, &count))
  {
    free(ELIST);
    return -1; // Failure of this method means the map does not exist.
  }
  int i;
  // We only care about climbing for horizontal movement
  const char canClimb = dZ == 0 && !HasBoolFlag(E, BFLAG_STATIC) && HasBoolFlag(E, BFLAG_CLIMBER);
  char frontBlocked = 0;
  char topBlocked = !inRange(z + 1, 0, MAP_DEPTH - 1);
  char aboveBlocked = topBlocked;
  if (E->destroyed)
  {
    free(ELIST);
    return -1;
  }
  for (i = 0; i < count; i++)
  {
    if (E->destroyed)
    {
      free(ELIST);
      return -1; // Lol.
    }
    int Z = GetEntZ(ELIST[i]);
    if (Z == z && !frontBlocked)
    {
      if (Collide(E, ELIST[i], dX, dY, dZ))
        frontBlocked = 1;
      continue;
    }
    if (canClimb && Z == z + 1 && !topBlocked)
    {
      if (Collide(E, ELIST[i], dX, dY, dZ))
        topBlocked = 1;
    }
  }
  if (frontBlocked && canClimb && !aboveBlocked)
  {
    ENTITY** ELISTSAMEPOS;
    int countSamePos = 0;
    if (!GetEntitiesOnPosition(map, curX, curY, &ELISTSAMEPOS, &countSamePos))
    {
      free(ELIST);
      free(ELISTSAMEPOS);
      return -1;
    }
    for (i = 0; i < countSamePos; i++)
    {
      int Z, u;
      int* EPos;
      GetDataFlag(ELISTSAMEPOS[i], FLAG_POS, (void**)&EPos);
      ConvertToZXY(*EPos, &Z, &u, &u);
      if (Z == z + 1 && Collide(E, ELISTSAMEPOS[i], dX, dY, dZ))
      {
        aboveBlocked = 1;
        break;
      }
    }
    free(ELISTSAMEPOS);
  }
  if (E->destroyed)
  {
    free(ELIST);
    return -1;
  }
  if (frontBlocked)
  {
    if (!canClimb || topBlocked || aboveBlocked)
    {
      return -1;
    }
    z++;
  }
  free(ELIST);
  MoveEnt(E, x, y);
  SetEntZ(E, z);
  return ScaleTime(TIMEOF_MOVE, E);
}

/**
 * Only supports a unit dZ.
 */
extern int TryMoveVertical(ENTITY* E, const int dZ)
{
  int rDz = dZ;
  return TryMove3D(E, 0, 0, rDz);
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
  return TryMove3D(E, desiredDX, desiredDY, 0);
}

/**
 * Shoot a damaging ray in a direction.
 * It will attempt to expend all its damage.
 * A range less than 0 disables the range check.
 * (SX, SY, SZ) and (EX, EY, EZ) are clamped to the map
 */
extern void DamageRay(ENTITY* map, int Range, int Damage, int SX, int SY, int SZ, int EX, int EY, int EZ)
{
  if (Damage == 0)
    return;
  SX = clamp(SX, 0, MAP_WIDTH - 1);
  SY = clamp(SY, 0, MAP_HEIGHT - 1);
  SZ = clamp(SZ, 0, MAP_DEPTH - 1);
  int DX = EX - SX;
  int DY = EY - SY;
  int DZ = EZ - SZ;
  int count;
  int** Line = createLine(DX, DY, DZ, 0, &count);
  int i = 0;
  int ECount;
  ENTITY** Array = 0;
  char RefreshArray = 0;
  for (i = 0; i < count; i++)
  {
    if (!InBounds3D(Line[i][0] + SX, Line[i][1] + SY, Line[i][2] + SZ))
      break;
    if (Damage == 0)
      break;
    if (Range > 0)
    {
      int Dist = Line[i][0] * Line[i][0] + Line[i][1] * Line[i][1] + Line[i][2] * Line[i][2];
      if (Dist > Range * Range + 1)
        break;
    }
    if (i == 0)
    {
      GetEntitiesOnPosition(map, Line[i][0] + SX, Line[i][1] + SY, &Array, &ECount);
    }
    else if (RefreshArray || Line[i][0] != Line[i - 1][0] || Line[i][1] != Line[i - 1][1])
    {
      free(Array);
      GetEntitiesOnPosition(map, Line[i][0] + SX, Line[i][1] + SY, &Array, &ECount);
      RefreshArray = 0;
    }
    entPrefab(PREFAB_EXPLOSIONGFX, map, Line[i][0] + SX, Line[i][1] + SY, Line[i][2] + SZ);
    int j;
    for (j = 0; j < ECount; j++)
    {
      if (Damage == 0)
        break;
      if (GetEntZ(Array[j]) != Line[i][2] + SZ)
        continue;
      if (!HasDataFlag(Array[j], FLAG_HEALTH))
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
extern void Explode(ENTITY* map, const int Range, const int Power, const int X, const int Y, const int Z)
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
        DamageRay(map, Range, Power, X, Y, Z, X + TX, Y + TY, Z + TZ);
        DamageRay(map, Range, Power, X, Y, Z, X + TZ, Y + TX, Z + TY);
        DamageRay(map, Range, Power, X, Y, Z, X + TY, Y + TZ, Z + TX);
      }
    }
  }
  for (TX = Range; TX <= Range; flipper(&TX))
  {
    for (TY = Range; TY <= Range; flipper(&TY))
    {
      for (TZ = Range; TZ <= Range; flipper(&TZ))
      {
        DamageRay(map, Range, Power, X, Y, Z, X + TX, Y + TY, Z + TZ);
      }
    }
  }
}

/**
 * Returns 0 if it cannot create a bullet.
 */
extern ENTITY* ShootBullet(ENTITY* shooter, const int Damage, int DX, int DY, int DZ)
{
  if (DX == 0 && DY == 0 && DZ == 0)
    return 0;
  int DXAbs = abs(DX);
  int DYAbs = abs(DY);
  int DZAbs = abs(DZ);
  int DTotal = DXAbs + DYAbs + DZAbs;
  int XV = (DTotal == DXAbs) ? DX : ((DTotal - DXAbs) * sign(DX));
  int YV = (DTotal == DYAbs) ? DY : ((DTotal - DYAbs) * sign(DY));
  int ZV = (DTotal == DZAbs) ? DZ : ((DTotal - DZAbs) * sign(DZ));
  int X, Y, Z;
  uint *Pos;
  GetDataFlag(shooter, FLAG_POS, (void**)&Pos);
  ConvertToZXY(*Pos, &Z, &X, &Y);
  ENTITY *map = mapOf(shooter);
  ENTITY* LeBoolet = entPrefab(PREFAB_BULLET, map, X, Y, Z);
  SetDataFlag(LeBoolet, FLAG_BULLETINFO, int2ap(Damage, 0));
  int XVAbs = abs(XV);
  int YVAbs = abs(YV);
  int ZVAbs = abs(ZV);
  float CurSpeed = (XVAbs ? (100.0 / XVAbs) : 0) + (YVAbs ? (100.0 / YVAbs) : 0) + (ZVAbs ? (100.0 / ZVAbs) : 0);
  static const int BulletVelocity = 50;
  static const float BulletVelInv = 1.0 / (float)BulletVelocity;
  if (DZAbs > DXAbs * 2 && DZAbs > DYAbs * 2)
  {
    entSetText(LeBoolet, '*');
  }
  else
  {
    if (DXAbs > DYAbs * 3)
    {
      entSetText(LeBoolet, '-');
    }
    else if (DYAbs > DXAbs * 3)
    {
      entSetText(LeBoolet, '|');
    }
    else
    {
      if (sign(DX) == sign(DY))
      {
        entSetText(LeBoolet, '\\');
      }
      else
      {
        entSetText(LeBoolet, '/');
      }
    }
  }
  XV *= CurSpeed;
  XV *= BulletVelInv;
  YV *= CurSpeed;
  YV *= BulletVelInv;
  ZV *= CurSpeed;
  ZV *= BulletVelInv;
  addStepper(LeBoolet, XV, YV, ZV, -1);
  return LeBoolet;
}

extern void TryJump(ENTITY* E)
{
  int* JumpHeight;
  GetDataFlag(E, FLAG_CANJUMP, (void**)&JumpHeight);
  if (!JumpHeight)
    return;
  int EntZ, EntX, EntY;
  uint* EntPos;
  GetDataFlag(E, FLAG_POS, (void**)&EntPos);
  if (!EntPos)
    return; // WOT
  ConvertToZXY(*EntPos, &EntZ, &EntX, &EntY);
  if (EntZ == 0)
  {
    SetDataFlag(E, FLAG_ZVEL, intap(-*JumpHeight));
    return;
  }
  ENTITY** map;
  GetDataFlag(E, FLAG_CONTAINEDBY, (void**)&map);
  if (!map) // no collision checks can be made at this time
    return;
  ENTITY** ELIST;
  int Count;
  GetEntitiesOnPosition(*map, EntX, EntY, &ELIST, &Count);
  int i;
  for (i = 0; i < Count; i++)
  {
    ENTITY* EBELOW = ELIST[i];
    if (GetEntZ(EBELOW) != EntZ - 1)
      continue;
    if (Collide(E, EBELOW, 0, 0, 1))
    {
      SetDataFlag(E, FLAG_ZVEL, intap(-*JumpHeight));
      break;
    }
  }
  free(ELIST);
}

/**
 * Handles controller logic. Called every game loop for every entity with a controller.
 * @param controller The controller object
 * @param game The game object
 * @return If anything happened
 */
extern char ControllerProcess(ENTITY_CONTROLLER* controller, const ENTITY* game)
{
  char hi = ';';
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
      int i, j;
      int* StartIndForZ = calloc(MAP_DEPTH, sizeof(int));
      int* EndIndForZ = calloc(MAP_DEPTH, sizeof(int));
      controller->nextAct += stepSize;
      for (i = 0; i < MAP_CELLCOUNT; i++)
      {
        const int X = i % MAP_WIDTH;
        const int Y = i / MAP_WIDTH;
        ENTITY** ELIST = GetEntsAtPos(X, Y);
        int len = 0;
        unsigned char* EPOS;
        ENTITY** MapOf;
        for (; ELIST[len]; len++);
        EPOS = calloc(len, sizeof(char));
        MapOf = calloc(len, sizeof(ENTITY*));

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
                if (Collide(FallingEntity, EntityBelow, 0, 0, -1))
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
            if (FallingEntity->destroyed)
              break; // Lawl.
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
  case CONT_DECAY:
    {
      int* decayInfo = 0;
      GetDataFlag(controller->associate, FLAG_DECAY, (void**)&decayInfo);
      if (!decayInfo)
      {
        // what are we even doing here
        ClearDataFlag(controller->associate, FLAG_CONTROLS);
        return 0;
      }

      float Doneness = (float)(decayInfo[0]) / (float)(decayInfo[1]);
      int DT = GLOBAL_TIMER - controller->nextAct + 1;
      decayInfo[0] -= DT;
      if (decayInfo[0] <= 0)
      {
        BreakEntity(controller->associate);
        return 0;
      }

      int NewFGColor, NewBGColor;
      if (HasDataFlag(controller->associate, FLAG_DECAYCOLOR))
      {
        int *DecayColor;
        GetDataFlag(controller->associate, FLAG_DECAYCOLOR, (void**)&DecayColor);
        NewFGColor = colorOf(Doneness * DecayColor[0], Doneness * DecayColor[1], Doneness * DecayColor[2]);
        NewBGColor = colorOf(Doneness * DecayColor[3], Doneness * DecayColor[4], Doneness * DecayColor[5]);
      }
      else
      {
        NewFGColor = colorOf(Doneness * 255, Doneness * 255, Doneness * 255);
        NewBGColor = colorOf(Doneness * 255, Doneness * 255, Doneness * 255);
      }
      entSetColor(controller->associate, NewFGColor, NewBGColor, 255);
      controller->nextAct = GLOBAL_TIMER + 1;
      return 0;
    }
  case CONT_STEPPER:
    {
      int *Stepperinfo, *Airtime;
      ENTITY* E = controller->associate;
      GetDataFlag(E, FLAG_STEPPERINFO, (void**)&Stepperinfo);
      GetDataFlag(E, FLAG_STEPPERAIRTIME, (void**)&Airtime);

      if (!Stepperinfo) // what in the god damn
      {
        ClearDataFlag(E, FLAG_CONTROLS);
        break;
      }

      uint* Pos;
      GetDataFlag(E, FLAG_POS, (void**)&Pos);
      int X, Y, Z;
      ConvertToZXY(*Pos, &Z, &X, &Y);

      int XSign = sign(Stepperinfo[1]);
      int YSign = sign(Stepperinfo[3]);
      int ZSign = sign(Stepperinfo[5]);

      int TargetDT = 10000;

      if (Stepperinfo[1])
        mini(&TargetDT, abs(Stepperinfo[1]));
      if (Stepperinfo[3])
        mini(&TargetDT, abs(Stepperinfo[3]));
      if (Stepperinfo[5])
        mini(&TargetDT, abs(Stepperinfo[5]));

      if (Airtime)
      {
        mini(&TargetDT, *Airtime);
        *Airtime -= TargetDT;
        if (*Airtime <= 0)
        {
          ClearBoolFlag(E, BFLAG_STATIC);
          ClearDataFlag(E, FLAG_STEPPERAIRTIME);
        }
      }

      Stepperinfo[0] += TargetDT;
      Stepperinfo[2] += TargetDT;
      Stepperinfo[4] += TargetDT;

      int dX, dY, dZ = dY = dX = 0;

      if (XSign && Stepperinfo[0] > abs(Stepperinfo[1]))
      {
        Stepperinfo[0] -= abs(Stepperinfo[1]);
        dX = XSign;
      }
      if (YSign && Stepperinfo[2] > abs(Stepperinfo[3]))
      {
        Stepperinfo[2] -= abs(Stepperinfo[3]);
        dY = YSign;
      }
      if (ZSign && Stepperinfo[4] > abs(Stepperinfo[5]))
      {
        Stepperinfo[4] -= abs(Stepperinfo[5]);
        dZ = ZSign;
      }

      TryMove3D(E, dX, dY, dZ);

      if (!InBounds3D(X + dX, Y + dY, Z + dZ))
      {
        BreakEntity(controller->associate);
        return 0;
      }

      if (!E->destroyed)
        controller->nextAct += TargetDT;

      return 1;
    }
  }
  return 0;
}
