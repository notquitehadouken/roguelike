/**
 * Handles rendering and shadowcasting.
 */

#pragma once
#include <string.h>

#include "global.h"
#include "keyhandler.h"
#include "object.h"
#ifdef USING_WINDOWS
#define <windows.h>
#endif
#ifdef USING_CONIO
#define <conio.h>
#endif
#define B_DEFAULT_TEXT ' '
#define B_DEFAULT_COLOR 255

struct PIXEL
{
  unsigned char text;
  unsigned char color;
  unsigned char backcolor;
  unsigned char renderorder;
};

typedef struct PIXEL B_PIXEL;

B_PIXEL PIXEL_DEFAULT = {B_DEFAULT_TEXT, B_DEFAULT_COLOR, 0, 0};
B_PIXEL PIXEL_VISION_OBSCURED = {'*', 233, 0, 0};
B_PIXEL PIXEL_VISION_OUT_OF_RANGE = {'.', 233, 0, 0};

/**
 * Checks if two pixels are equal
 * @param a First pixel
 * @param b Second pixel
 * @return Well are they?
 */
extern attrpure char b_pixEq(const B_PIXEL* a, const B_PIXEL* b)
{
  if (a == b) return 1;
  if (!a || !b)
    return 0;
  return (a->text == b->text
    && a->color == b->color
    && a->backcolor == b->backcolor
    && a->renderorder == b->renderorder);
}

/**
 * Duplicates a pixel.
 */
extern attrmalloc B_PIXEL* b_pixDupe(const B_PIXEL* target)
{
  B_PIXEL *replacement = malloc(sizeof(B_PIXEL));
  replacement->text = target->text;
  replacement->color = target->color;
  replacement->backcolor = target->backcolor;
  replacement->renderorder = target->renderorder;
  return replacement;
}

/**
 * A screen buffer.
 */
struct BUFFER
{
  char initialized;
  B_PIXEL* array[S_LENGTH];
  short depth[S_LENGTH];
  char effect[S_LENGTH];
};

typedef struct BUFFER B_BUFFER;

/**
 * Properly writes a pixel to a buffer
 * @param buffer The buffer
 * @param index The index
 * @param pixel The pixel
 */
extern void b_writePixel(B_BUFFER* buffer, const int index, B_PIXEL* pixel)
{
  if (!buffer->initialized)
    return;
  if (b_pixEq(buffer->array[index], pixel))
    return;
  if (buffer->array[index])
    free(buffer->array[index]);
  pixel = b_pixDupe(pixel);
  buffer->array[index] = pixel;
}

/**
 * Sets the initialized flag to true and forces every pixel in a buffer to be Target.
 * @param buffer The buffer
 * @param Target The pixel
 */
extern void b_factoryWith(B_BUFFER* buffer, B_PIXEL* Target)
{
  int iter;
  for (iter = 0; iter < S_LENGTH; iter++)
  {
    b_writePixel(buffer, iter, Target);
    buffer->depth[iter] = 0;
    buffer->effect[iter] = 0;
  }
  buffer->initialized = 1;
}

/**
 * Sets the initialized flag to true and forces every pixel in a buffer to be PIXEL_DEFAULT.
 * @param buffer The buffer
 */
extern void b_factory(B_BUFFER* buffer)
{
  b_factoryWith(buffer, &PIXEL_DEFAULT);
}

/**
 * Sets an entire row of a buffer to the default pixel.
 */
extern void b_clearRow(B_BUFFER* buffer, const uint row)
{
  if (row >= S_ROW)
    return; // ok johnny segfault

  int iter;
  for (iter = 0; iter < S_COL; iter++)
  {
    int ind = iter + row * S_COL;
    b_writePixel(buffer, ind, &PIXEL_DEFAULT);
    buffer->depth[ind] = 0;
    buffer->effect[ind] = 0;
  }
}

/**
 * Destroys a buffer
 * @param buffer The buffer
 */
extern void b_discard(B_BUFFER* buffer)
{
  if (!buffer->initialized) return; // What?
  free(buffer);
}

/**
 * Initializes a buffer
 * @param buffer The buffer
 */
extern void b_initialize(B_BUFFER** buffer)
{
  *buffer = (B_BUFFER*)malloc(sizeof(B_BUFFER));
  int iter;
  for (iter = 0; iter < S_LENGTH; iter++)
  {
    (*buffer)->array[iter] = 0;
    (*buffer)->depth[iter] = 0;
    (*buffer)->effect[iter] = 0;
  }
  (*buffer)->initialized = 1;
  b_factory(*buffer);
}

/**
 * Returns the proper index for a buffer
 * @param row The row (Y value)
 * @param col The column (X value)
 * @return The index
 */
extern attrpure unsigned long b_getIndex(const int row, const int col)
{
  // so i don't need to write row * S_COL + col 5000 times
  return row * S_COL + col;
}

/**
 * Gets a pixel from a buffer
 * @param buffer The buffer
 * @param row The row (Y value)
 * @param col The column (X value)
 * @param out The pixel
*/
extern void b_getPixel(const B_BUFFER* buffer, const int row, const int col, B_PIXEL** out)
{
  *out = buffer->array[b_getIndex(row, col)];
}

extern short b_getDepth(const B_BUFFER* buffer, const int row, const int col)
{
  return buffer->depth[b_getIndex(row, col)];
}

extern char b_getEffect(const B_BUFFER* buffer, const int row, const int col)
{
  return buffer->effect[b_getIndex(row, col)];
}

/**
 * Sets a pixel to a buffer
 * @param buffer The buffer
 * @param row The row (Y value)
 * @param col The column (X value)
 * @param pixel The pixel
*/
extern void b_setPixel(B_BUFFER* buffer, const int row, const int col, B_PIXEL* pixel)
{
  b_writePixel(buffer, b_getIndex(row, col), pixel);
}

extern void b_setDepth(B_BUFFER* buffer, const int row, const int col, const short depth)
{
  buffer->depth[b_getIndex(row, col)] = depth;
}

extern void b_setEffect(B_BUFFER* buffer, const int row, const int col, const char effect)
{
  buffer->effect[b_getIndex(row, col)] = effect;
}

/**
 * Writes some colorful text to a buffer
 * @param buffer The buffer
 * @param row The row (Y value)
 * @param col The column (X value)
 * @param text The text
 * @param color The color
 * @param backcolor Background color
 */
extern void b_writeTwoColor(B_BUFFER* buffer, int row, const int col, const char* text,
                            const unsigned char color, const unsigned char backcolor)
{
  int index = b_getIndex(row, col);
  int start = 0;
  int i;
  for (i = 0; text[i]; i++)
  {
    if (text[i] == '\n')
    {
      index = b_getIndex(++row, col);
      start = i + 1;
      continue;
    }
    if (i > S_LENGTH)
      break;
    B_PIXEL* P = malloc(sizeof(B_PIXEL));
    P->text = text[i];
    P->color = color;
    P->backcolor = backcolor;
    P->renderorder = 0;
    b_writePixel(buffer, index + i - start, P);
    free(P);
    b_setDepth(buffer, row, col, 0);
    b_setEffect(buffer, row, col, 0);
  }
}

extern void b_writeToColor(B_BUFFER* buffer, const int row, const int col, const char* text, const unsigned char color)
{
  b_writeTwoColor(buffer, row, col, text, color, 0);
}

/**
* Writes some default-color text to a buffer
 * @param buffer The buffer
 * @param row The row (Y value)
 * @param col The column (X value)
 * @param text The text
 */
extern void b_writeTo(B_BUFFER* buffer, const int row, const int col, const char* text)
{
  b_writeToColor(buffer, row, col, text, B_DEFAULT_COLOR);
}

/**
 * Puts the cursor somewhere.
 * Used in printing
 * @param row The row (Y value)
 * @param col The column (X value)
 */
extern void s_putCursor(const int row, const int col)
{
  fprintf(stdout, "\033[%i;%iH", row + 1, col + 1);
}

/**
 * Clears the entire screen.
 * For windows, also sets necessary terminal flags.
 */
extern void s_clearScreen()
{
#ifdef USING_WINDOWS
  HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
  DWORD dwMode;
  GetConsoleMode(hOutput, &dwMode);
  dwMode |= ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
  SetConsoleMode(hOutput, dwMode);
#endif
  fputs("\033[3J\033[2J", stdout);
  fflush(stdout);
}

enum MAPCOLORSCALING
{
  COLORSCALING_STANDARD = 0,
  COLORSCALING_LOW = 1
};

int mapColorScaling = COLORSCALING_STANDARD;

B_BUFFER* curbuffer = {0};

/**
 * Draws a buffer to the screen
 * @param buffer The buffer
 */
extern void b_draw(const B_BUFFER* buffer)
{
  if (curbuffer == NULL || !curbuffer->initialized)
  {
    s_clearScreen();
    b_initialize(&curbuffer);
  }
  int row;
  for (row = 0; row < S_ROW; row++)
  {
    // Last checked: Maximum byte count per pixel of 30
    // 64 * S_COL is ~6 kb
    char* sBuffer = calloc(64 * S_COL, sizeof(char));
    char lastChanged = 0;
    int lastBufferEnd = 0;
    int lastColor = -256;
    int lastBackColor = -256;
    int lastEffect = 0;
    int lastDepth = -256;
    int writeColumn = -1;
    int col;
    for (col = 0; col < S_COL; col++)
    {
      B_PIXEL *pixel, *curpixel;
      b_getPixel(buffer, row, col, &pixel);
      b_getPixel(curbuffer, row, col, &curpixel);
      int depth = b_getDepth(buffer, row, col);
      int curdepth = b_getDepth(curbuffer, row, col);
      int effect = b_getEffect(buffer, row, col);
      int cureffect = b_getEffect(curbuffer, row, col);
      if (pixel->text < 32 || pixel->text >= 127)
        pixel->text = 32; // Kiss my A S S !
      if (b_pixEq(pixel, curpixel) && depth == curdepth && effect == cureffect)
      {
        if (lastChanged)
        {
          s_putCursor(row, writeColumn);
          lastChanged = 0;
          writeColumn = -1;
          fputs(sBuffer + lastBufferEnd, stdout);
          lastBufferEnd = strlen(sBuffer);
        }
        continue;
      }
      lastChanged = 1;
      if (writeColumn == -1)
      {
        writeColumn = col;
      }
      if (lastColor != pixel->color
        || lastBackColor != pixel->backcolor
        || lastDepth != depth
        || lastEffect != effect)
      {
        lastColor = pixel->color;
        lastBackColor = pixel->backcolor;
        lastDepth = depth;
        lastEffect = effect;
        // Maximum byte count: 30
        unsigned char fR;
        unsigned char fG;
        unsigned char fB;
        unsigned char bR;
        unsigned char bG;
        unsigned char bB;
        colorOf8Bit(lastColor, &fR, &fG, &fB);
        colorOf8Bit(lastBackColor, &bR, &bG, &bB);
        if (depth > 0)
        {
          float scale = 1;
          if (mapColorScaling == COLORSCALING_LOW)
            scale = 4;
          float colorScaling = 1 -
            (float)depth / ((float)depth + scale);
          fR *= colorScaling;
          fG *= colorScaling;
          fB *= colorScaling;
          bR *= colorScaling;
          bG *= colorScaling;
          bB *= colorScaling;
        }
        else if (depth < 0)
        {
          float scale = 4;
          if (mapColorScaling == COLORSCALING_LOW)
            scale = 12;
          float colorScaling = 1 -
            -(float)depth / (-(float)depth + scale);
          fR = 255 - (255 - fR) * colorScaling;
          fG = 255 - (255 - fG) * colorScaling;
          fB = 255 - (255 - fB) * colorScaling;
          bR = 255 - (255 - bR) * colorScaling;
          bG = 255 - (255 - bG) * colorScaling;
          bB = 255 - (255 - bB) * colorScaling;
        }
        sprintf(sBuffer + strlen(sBuffer), "\033[0m");
        if (lastBackColor || depth < 0)
          sprintf(sBuffer + strlen(sBuffer), "\033[48;2;%d;%d;%dm", bR, bG, bB);
        sprintf(sBuffer + strlen(sBuffer), "\033[38;2;%d;%d;%dm", fR, fG, fB);
        if (lastEffect)
          sprintf(sBuffer + strlen(sBuffer), "\033[%dm", lastEffect);
      }
      sprintf(sBuffer + strlen(sBuffer), "%c", pixel->text);
      b_setPixel(curbuffer, row, col, pixel);
      b_setDepth(curbuffer, row, col, depth);
      b_setEffect(curbuffer, row, col, effect);
    }
    if (lastChanged)
    {
      s_putCursor(row, writeColumn);
      fputs(sBuffer + lastBufferEnd, stdout);
    }
    free(sBuffer);
    fputs("\033[0m", stdout);
  }
  s_putCursor(S_ROW, S_COL);
  fputs("\033[0m", stdout);
  fflush(stdout);
}

/**
 * Forces a full redraw of a buffer
 * @param buffer The buffer
 */
extern void b_flush(const B_BUFFER* buffer)
{
  // Triggers a full redraw of the screen
  if (curbuffer == NULL || !curbuffer->initialized)
  {
    b_initialize(&curbuffer);
  }
  b_factory(curbuffer);
  s_clearScreen();
  b_draw(buffer);
}

extern attrpure char b_objOccludes(const ENTITY* thelooker, const ENTITY* obj)
{
  if (HasBoolFlag(obj, BFLAG_OCCLUDING))
    return 1;
  if (HasBoolFlag(obj, BFLAG_OCCLUDESFAR))
  {
    int lX, lY, lZ, oX, oY, oZ;
    uint *lPos, *oPos;
    GetDataFlag(thelooker, FLAG_POS, (void**)&lPos);
    GetDataFlag(obj, FLAG_POS, (void**)&oPos);
    if (lPos && oPos)
    {
      ConvertToZXY(*lPos, &lZ, &lX, &lY);
      ConvertToZXY(*oPos, &oZ, &oX, &oY);
      if (mag(lZ - oZ) > 1 || mag(lX - oX) > 1 || mag(lY - oY) > 1)
      {
        return 1;
      }
    }
  }
  return 0;
}

unsigned char* occludedBuffer = 0; // Do not access this.
ENTITY** occlusionMapContents = 0;
char occludedDir[27];
int occludedLastX = -1, occludedLastY = -1, occludedLastZ = -1, occludedLastSightRange = 0;

#ifndef OCCLUSION_THREADCOUNT
#define OCCLUSION_THREADCOUNT 16
#endif

struct Occlusion_StaticInfo
{
  int PlayerX;
  int PlayerY;
  int PlayerZ;
  int LowX;
  int LowY;
  int LowZ;
  int HighX;
  int HighY;
  int HighZ;
  int ViewRange;
  const ENTITY *Player;
};

struct Occlusion_StaticInfo occludedStaticInfo;

pthread_barrier_t Barrier_BeginBufferload;
pthread_barrier_t Barrier_EndBufferload;
pthread_barrier_t Barrier_BeginLOS;
pthread_barrier_t Barrier_EndLOS;

extern __attribute__ ((__noreturn__)) void* __b_occludeThread(void* arg)
{
  int Index = *(int*)arg;
  int i;
  // ReSharper disable once CppDFAEndlessLoop
  for (;;)
  {
    pthread_barrier_wait(&Barrier_BeginBufferload);

    int PlayerX = occludedStaticInfo.PlayerX;
    int PlayerY = occludedStaticInfo.PlayerY;
    int PlayerZ = occludedStaticInfo.PlayerZ;
    int LowX = occludedStaticInfo.LowX;
    int LowY = occludedStaticInfo.LowY;
    int LowZ = occludedStaticInfo.LowZ;
    int HighX = occludedStaticInfo.HighX;
    int HighY = occludedStaticInfo.HighY;
    int HighZ = occludedStaticInfo.HighZ;
    int ViewRange = occludedStaticInfo.ViewRange;

    for (i = 0; occlusionMapContents[i]; i++)
    {
      if (i % OCCLUSION_THREADCOUNT != Index)
        continue;

      ENTITY* access = occlusionMapContents[i];
      if (!access)
        break;
      if (!HasDataFlag(access, FLAG_POS) || !b_objOccludes(occludedStaticInfo.Player, access))
        continue; //  you'd be hard pressed
      int x, y, z;
      uint* pos;
      GetDataFlag(access, FLAG_POS, (void**)&pos);
      ConvertToZXY(*pos, &z, &x, &y);
      if (!InBounds3D(x, y, z))
        continue;
      occludedBuffer[x + y * MAP_WIDTH + z * MAP_CELLCOUNT] |= 0b1;
      const int dX = x - occludedStaticInfo.PlayerX;
      const int dY = y - occludedStaticInfo.PlayerY;
      const int dZ = z - occludedStaticInfo.PlayerZ;
      if (dX == 0 && dY == 0 && dZ == 0)
        continue;
      const int dXs = sign(dX);
      const int dYs = sign(dY);
      const int dZs = sign(dZ);
      occludedDir[13 + dXs + dYs * 3 + dZs * 9] = 1;
      occludedDir[13 + dXs + dYs * 3] = 1;
      occludedDir[13 + dXs + dZs * 9] = 1;
      occludedDir[13 + dYs * 3 + dZs * 9] = 1;
      occludedDir[13 + dXs] = 1;
      occludedDir[13 + dYs * 3] = 1;
      occludedDir[13 + dZs * 9] = 1;
    }

    pthread_barrier_wait(&Barrier_EndBufferload);
    pthread_barrier_wait(&Barrier_BeginLOS);

    int accumulate = 0;
    int oX;
    for (oX = LowX; oX <= HighX; oX++)
    {
      int oY;
      for (oY = LowY; oY <= HighY; oY++)
      {
        int oZ;
        for (oZ = LowZ; oZ <= HighZ; oZ++)
        {
          if (accumulate++ % OCCLUSION_THREADCOUNT != Index)
            continue;
          const int dX = oX - PlayerX;
          const int dY = oY - PlayerY;
          const int dZ = oZ - PlayerZ;
          const int oIndex = oX + oY * MAP_WIDTH + oZ * MAP_CELLCOUNT;
          occludedBuffer[oIndex] &= 0b11;
          if (dX * dX + dY * dY + dZ * dZ >= ((float)(ViewRange * ViewRange)))
          {
            occludedBuffer[oIndex] |= 0b100;
            continue;
          }
          if (occludedDir[13 + sign(dX) + sign(dY) * 3 + sign(dZ) * 9] == 0)
          {
            occludedBuffer[oIndex] = 0;
            continue;
          }
          if (occludedBuffer[oIndex] & 0b10)
            continue;
          int count;
          int** line = createLine(dX, dY, dZ, 1, &count);
          for (i = 0; i < count - 1; i++)
          {
            const int lX = line[i][0] + PlayerX;
            const int lY = line[i][1] + PlayerY;
            const int lZ = line[i][2] + PlayerZ;
            const int lIndex = lX + lY * MAP_WIDTH + lZ * MAP_CELLCOUNT;
            if (occludedBuffer[lIndex] & 0b1)
            {
              occludedBuffer[oIndex] |= 0b10;
              break;
            }
          }
          freeLine(line, count);
        }
      }
    }

    pthread_barrier_wait(&Barrier_EndLOS);
  }
}

pthread_t *ThreadPool = 0;

/**
 * Calculates every position in a map that the player can see
 * @param map The map
 * @param player The player entity
 * @return A MAP_LENGTH * MAP_DEPTH list of characters. The 2s place is if it is occluded, the 1s place is if it occludes.
 */
extern unsigned char* b_getOccluded(const ENTITY* map, const ENTITY* player)
{
  if (!occludedBuffer)
  {
    occludedBuffer = calloc(MAP_CELLCOUNT * MAP_DEPTH, sizeof(*occludedBuffer));
    int i;
    for (i = 0; i < MAP_CELLCOUNT * MAP_DEPTH; i++)
      occludedBuffer[i] = 0b110;
  }
  if (!occludedBuffer)
    return 0;
  if (!ThreadPool)
  {
    // This thread must also wait at the barrier
    pthread_barrier_init(&Barrier_BeginBufferload, 0, OCCLUSION_THREADCOUNT + 1);
    pthread_barrier_init(&Barrier_EndBufferload, 0, OCCLUSION_THREADCOUNT + 1);
    pthread_barrier_init(&Barrier_BeginLOS, 0, OCCLUSION_THREADCOUNT + 1);
    pthread_barrier_init(&Barrier_EndLOS, 0, OCCLUSION_THREADCOUNT + 1);
    ThreadPool = calloc(OCCLUSION_THREADCOUNT, sizeof(pthread_t));
    int i;
    for (i = 0; i < OCCLUSION_THREADCOUNT; i++)
    {
      pthread_create(ThreadPool + i, 0, __b_occludeThread, intap(i));
    }
  }
  int i;
  for (i = 0; i < 27; i++)
    occludedDir[i] = 0;
  int PlayerZ, PlayerX, PlayerY;
  uint* playerPos;
  GetDataFlag(player, FLAG_POS, (void**)&playerPos);
  ConvertToZXY(*playerPos, &PlayerZ, &PlayerX, &PlayerY);
  _CACHE_OF(ENTITY)* container;
  GetDataFlag(map, FLAG_CONTAINER, (void**)&container);
  int* ViewRange = 0;
  GetDataFlag(player, FLAG_SIGHTRANGE, (void**)&ViewRange);
  if (!ViewRange)
  {
    int r = 9999;
    ViewRange = &r;
  }
  if (occludedLastX == -1)
    occludedLastX = PlayerX;
  if (occludedLastY == -1)
    occludedLastY = PlayerY;
  if (occludedLastZ == -1)
    occludedLastZ = PlayerZ;
  int ovX, ovY, ovZ;
  int LowX = min(occludedLastX - occludedLastSightRange, PlayerX - *ViewRange);
  int HighX = max(occludedLastX + occludedLastSightRange, PlayerX + *ViewRange);
  int LowY = min(occludedLastY - occludedLastSightRange, PlayerY - *ViewRange);
  int HighY = max(occludedLastY + occludedLastSightRange, PlayerY + *ViewRange);
  int LowZ = min(occludedLastZ - occludedLastSightRange, PlayerZ - *ViewRange);
  int HighZ = max(occludedLastZ + occludedLastSightRange, PlayerZ + *ViewRange);

  LowX = max(LowX, 0);
  HighX = min(HighX, MAP_WIDTH - 1);
  LowY = max(LowY, 0);
  HighY = min(HighY, MAP_HEIGHT - 1);
  LowZ = max(LowZ, 0);
  HighZ = min(HighZ, MAP_DEPTH - 1);

  for (ovX = LowX; ovX <= HighX; ovX++)
  {
    for (ovY = LowY; ovY <= HighY; ovY++)
    {
      for (ovZ = LowZ; ovZ <= HighZ; ovZ++)
      {
        occludedBuffer[ovX + ovY * MAP_WIDTH + ovZ * MAP_CELLCOUNT] = 0b100;
      }
    }
  }

  occludedLastX = PlayerX;
  occludedLastY = PlayerY;
  occludedLastZ = PlayerZ;
  occludedLastSightRange = *ViewRange;
  LowX = max(PlayerX - *ViewRange, 0);
  HighX = min(PlayerX + *ViewRange, MAP_WIDTH - 1);
  LowY = max(PlayerY - *ViewRange, 0);
  HighY = min(PlayerY + *ViewRange, MAP_HEIGHT - 1);
  LowZ = max(PlayerZ - *ViewRange, 0);
  HighZ = min(PlayerZ + *ViewRange, MAP_DEPTH - 1);

  occlusionMapContents = (ENTITY**)cacheAsList(_CACHE(*container));

  occludedStaticInfo.PlayerX = PlayerX;
  occludedStaticInfo.PlayerY = PlayerY;
  occludedStaticInfo.PlayerZ = PlayerZ;
  occludedStaticInfo.LowX = LowX;
  occludedStaticInfo.LowY = LowY;
  occludedStaticInfo.LowZ = LowZ;
  occludedStaticInfo.HighX = HighX;
  occludedStaticInfo.HighY = HighY;
  occludedStaticInfo.HighZ = HighZ;
  occludedStaticInfo.ViewRange = *ViewRange;
  occludedStaticInfo.Player = player;

  pthread_barrier_wait(&Barrier_BeginBufferload);
  pthread_barrier_wait(&Barrier_EndBufferload);

  int TX, TY, TZ;
  for (TX = 0; TX <= *ViewRange; flipper(&TX))
  {
    if (!inRange(TX, LowX, HighX))
      continue;
    char XSign = sign(TX);
    int XMag = abs(TX);
    int RX = TX + PlayerX;
    for (TY = 0; TY <= *ViewRange; flipper(&TY))
    {
      if (!inRange(TY, LowY, HighY))
        continue;
      char YSign = sign(TY);
      int YMag = abs(TY);
      int RY = TY + PlayerY;
      for (TZ = 0; TZ <= *ViewRange; flipper(&TZ))
      {
        if (!inRange(TZ, LowZ, HighZ))
          continue;
        char ZSign = sign(TZ);
        int ZMag = abs(TZ);
        int RZ = TZ + PlayerZ;

        int XToCenter = TX - XSign + PlayerX;
        int YToCenter = TY - YSign + PlayerY;
        int ZToCenter = TZ - ZSign + PlayerZ;
        if (!(occludedBuffer[XToCenter + YToCenter * MAP_WIDTH + ZToCenter * MAP_CELLCOUNT] & 0b11))
        {
          continue; // principal isn't occluded how joyous
        }

        char XBiggest = XMag >= ZMag && XMag >= YMag;
        char YBiggest = YMag >= XMag && YMag >= ZMag;
        char ZBiggest = ZMag >= XMag && ZMag >= YMag;
        if (!XBiggest)
        {
          if (!(occludedBuffer[RX + YToCenter * MAP_WIDTH + ZToCenter * MAP_CELLCOUNT] & 0b11))
            continue;
          if (!YBiggest &&
            !(occludedBuffer[RX + RY * MAP_WIDTH + ZToCenter * MAP_CELLCOUNT] & 0b11))
            continue;
        }
        if (!YBiggest)
        {
          if (!(occludedBuffer[XToCenter + RY * MAP_WIDTH + ZToCenter * MAP_CELLCOUNT] & 0b11))
            continue;
          if (!ZBiggest &&
            !(occludedBuffer[XToCenter + RY * MAP_WIDTH + RZ * MAP_CELLCOUNT] & 0b11))
            continue;
        }
        if (!ZBiggest)
        {
          if (!(occludedBuffer[XToCenter + YToCenter * MAP_WIDTH + RZ * MAP_CELLCOUNT] & 0b11))
            continue;
          if (!XBiggest &&
            !(occludedBuffer[RX + YToCenter * MAP_WIDTH + RZ * MAP_CELLCOUNT] & 0b11))
            continue;
        }
        occludedBuffer[RX + RY * MAP_WIDTH + RZ * MAP_CELLCOUNT] |= 0b10;
      }
    }
  }

  pthread_barrier_wait(&Barrier_BeginLOS);
  pthread_barrier_wait(&Barrier_EndLOS);

  free(occlusionMapContents);
  return occludedBuffer;
}

enum MAPVIEWHEIGHT
{
  VIEWHEIGHT_CUSTOM = -2,
  VIEWHEIGHT_LOW = -1,
  VIEWHEIGHT_LEVEL = 0,
  VIEWHEIGHT_HIGH = 1,
};

int mapViewHeight = VIEWHEIGHT_LEVEL;
int mapCustomVH = 0;

const ENTITY *entitiesDrawn[MAP_CELLCOUNT]; // Entities shown in the last drawn buffer.

/**
 * Draws an entire map to a buffer.
 * @param buffer The buffer
 * @param map The map
 * @param player The player
 */
extern void b_writeMapToBuffer(B_BUFFER* buffer, const ENTITY* map, const ENTITY* player)
{
  int drawn[MAP_CELLCOUNT];
  int prior[MAP_CELLCOUNT];
  int highest[MAP_CELLCOUNT];
  unsigned char* occluded = 0;
  if (player)
    occluded = b_getOccluded(map, player);
  int PlayerZ;
  int PlayerX;
  int PlayerY;
  uint* PlayerPos;
  GetDataFlag(player, FLAG_POS, (void**)&PlayerPos);
  ConvertToZXY(*PlayerPos, &PlayerZ, &PlayerX, &PlayerY);
  int i;
  for (i = 0; i < MAP_CELLCOUNT; i++)
  {
    entitiesDrawn[i] = 0;
    drawn[i] = -1;
    prior[i] = -1;
    if (occluded)
    {
      int j;
      highest[i] = -1;
      switch (mapViewHeight)
      {
      default:
      case VIEWHEIGHT_LEVEL:
        {
          for (j = MAP_DEPTH - 1; j >= 0; j--)
          {
            unsigned char occlusionMetric = occluded[j * MAP_CELLCOUNT + i];
            if (j < PlayerZ && highest[i] != -1)
            {
              break;
            }
            if (occlusionMetric & 0b110)
              continue;
            highest[i] = j;
          }
          break;
        }
      case VIEWHEIGHT_LOW:
        {
          for (j = 0; j < MAP_DEPTH; j++)
          {
            unsigned char occlusionMetric = occluded[j * MAP_CELLCOUNT + i];
            if (j >= PlayerZ - 1 && highest[i] != -1)
              break;
            if (highest[i] != -1 && occlusionMetric)
            {
              highest[i] = j - 1;
              break;
            }
            if (!(occlusionMetric & 0b110))
            {
              highest[i] = j;
            }
          }
          break;
        }
      case VIEWHEIGHT_HIGH:
        {
          for (j = MAP_DEPTH - 1; j >= 0; j--)
          {
            unsigned char occlusionMetric = occluded[j * MAP_CELLCOUNT + i];
            if (occlusionMetric & 0b110)
              continue;
            highest[i] = j;
            break;
          }
          break;
        }
      case VIEWHEIGHT_CUSTOM:
        {
          for (j = 0; j < MAP_DEPTH; j++)
          {
            highest[i] = mapCustomVH;
          }
          break;
        }
      }
    }
  }
  b_factory(buffer);
  _CACHE_OF(ENTITY)* ELIST;
  GetDataFlag(map, FLAG_CONTAINER, (void**)&ELIST);
  if (!ELIST) // ok sure
    return;
  uint listLength = cacheLength(_CACHE(*ELIST));
  for (i = 0; i < listLength; i++)
  {
    const ENTITY* ent = _CACHE_ACCESS(*ELIST, i);
    if (!ent)
      break; // well uhh fhuck me i suppose
    if (HasBoolFlag(ent, BFLAG_NORENDER))
      continue;
    int z, x, y;
    uint* posDat;
    GetDataFlag(ent, FLAG_POS, (void**)&posDat);
    if (!posDat)
      continue; // hwat
    ConvertToZXY(*posDat, &z, &x, &y);
    int rX = x;
    int rY = y;
#if defined IHTS_CHREE_DEEH
    x -= PlayerX;
    y -= PlayerY;
    y -= z - PlayerZ;
    x += MAP_WIDTH / 2;
    y += MAP_HEIGHT / 2;
#endif
    if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT || z < 0 || z >= MAP_DEPTH)
      continue;
    const int index = x + y * MAP_WIDTH;
    if (highest[rX + rY * MAP_WIDTH] < z)
      continue;
    int occlusionMetric = 0;
    if (occluded)
    {
      occlusionMetric = occluded[rX + rY * MAP_WIDTH + z * MAP_CELLCOUNT];
    }
    if (occlusionMetric & 0b110)
    {
      if (drawn[index] == -1)
      {
        b_setPixel(buffer, y + 2, x,
                   (occlusionMetric & 0b100) ? &PIXEL_VISION_OUT_OF_RANGE : &PIXEL_VISION_OBSCURED);
        b_setDepth(buffer, y + 2, x, 0);
      }
      continue;
    }
    if (drawn[index] > z)
      continue;
    B_PIXEL* pixel;
    GetDataFlag(ent, FLAG_APPEARANCE, (void**)&pixel);
    if (!pixel)
      continue; // WHAT
    if (drawn[index] == z && pixel->renderorder < prior[index])
      continue;
    drawn[index] = z;
    prior[index] = pixel->renderorder;
    entitiesDrawn[index] = ent;
    b_setPixel(buffer, y + 2, x, pixel);
    short depth = PlayerZ - z;
    if (mapViewHeight == VIEWHEIGHT_CUSTOM)
      depth = mapCustomVH - z;
    b_setDepth(buffer, y + 2, x, depth);
    b_setEffect(buffer, y + 2, x, 0);
  }
}

/**
 * Draws various snippets of information about the player to a buffer
 * @param buffer The buffer
 * @param player The player
 */
extern void b_writeHudToBuffer(B_BUFFER* buffer, const ENTITY* player)
{
  int* HP;

  GetDataFlag(player, FLAG_HEALTH, (void**)&HP);

  int xCur = 0;

  int viewIsLow = mapViewHeight == VIEWHEIGHT_LOW;
  int viewIsLevel = mapViewHeight == VIEWHEIGHT_LEVEL;
  int viewIsHigh = mapViewHeight == VIEWHEIGHT_HIGH;

  if (mapViewHeight == VIEWHEIGHT_CUSTOM)
  {
    int diff = mapCustomVH - GetEntZ(player);
    char text[MAX_STR_LEN];
    sprintf(text, "%+d", diff);
    b_writeTo(buffer, S_ROW - 2, xCur + 3 - strlen(text), text);
  }
  else
  {
    b_writeTwoColor(buffer, S_ROW - 2, xCur + 2, "^",
                    viewIsHigh ? 0 : 255, viewIsHigh ? 255 : 0);
    b_writeTwoColor(buffer, S_ROW - 2, xCur + 1, "=",
                    viewIsLevel ? 0 : 255, viewIsLevel ? 255 : 0);
    b_writeTwoColor(buffer, S_ROW - 2, xCur, "v",
                    viewIsLow ? 0 : 255, viewIsLow ? 255 : 0);
  }

  xCur += 4;

  char TimeText[MAX_STR_LEN];
  sprintf(TimeText, "Time: %.2f", GLOBAL_TIMER * 0.01);
  b_writeTo(buffer, S_ROW - 2, xCur, TimeText);
  xCur += strlen(TimeText) + 1;

  char Teckst[MAX_STR_LEN];
  int *Dont;
  GetDataFlag(player, FLAG_ZVEL, (void**)&Dont);
  sprintf(Teckst, "Z-Vel: %+d", Dont ? -*Dont : 0);
  b_writeTo(buffer, S_ROW - 2, xCur, Teckst);

  xCur = 0;

  if (HP)
  {
    char* HPText = calloc(MAX_STR_LEN, sizeof(char));
    sprintf(HPText, "%d", HP[1]);
    int HPLen = strlen(HPText);
    free(HPText);
    HPText = calloc(HPLen * 2 + 4, sizeof(char));
    sprintf(HPText, "%d", HP[0]);
    int i;
    for (i = 0; i <= HPLen; i++)
    {
      if (HPText[i] <= ' ')
        HPText[i] = ' ';
    }
    sprintf(HPText + HPLen, " / %d", HP[1]);

    unsigned char color;
    unsigned char backcolor = 0;
    const int ratio = HP[0] * 5 / HP[1];
    switch (ratio)
    {
    case 0:
      {
        color = 232;
        backcolor = 255;
        break;
      }
    case 1:
      {
        color = 1;
        break;
      }
    case 2:
      {
        color = 3;
        break;
      }
    case 3:
      {
        color = 6;
        break;
      }
    case 4:
      {
        color = 2;
        break;
      }
    default:
      {
        color = 4;
        break;
      }
    }
    b_writeTwoColor(buffer, S_ROW - 1, xCur, HPText, color, backcolor);
    xCur += 3 + HPLen * 2 + 4;

    free(HPText);
  }

  char SpeedText[MAX_STR_LEN];
  sprintf(SpeedText, "%d", ScaleTime(TIMEOF_MOVE, player));
  int xTo = 0;
  while (SpeedText[++xTo]);

  unsigned char color;
  unsigned char backcolor = 0;

  int digits = -1;
  int timeOf = ScaleTime(0b1000, player);
  while (timeOf >> ++digits);
  switch (digits)
  {
  case 0: // 12 or less
    color = 15;
    backcolor = 53;
    break;
  case 1: // 13+
    color = 5;
    break;
  case 2: // 25+
    color = 4;
    break;
  case 3: // 50+
    color = 6;
    break;
  case 4: // 100+
    color = 2;
    break;
  case 5: // 200+
    color = 11;
    break;
  default:
  case 6: // 400+
    color = 1;
    break;
  }

  b_writeToColor(buffer, S_ROW - 1, xCur, "MT: ", digits > 6 ? 1 : B_DEFAULT_COLOR);
  b_writeTwoColor(buffer, S_ROW - 1, xCur + 4, SpeedText, color, backcolor);

  // xCur += xTo + 4;
}

/* Not in keyhandler.h because of reliance on screen.h defined functions
 * keyhandler.h is included before screen.h
 */
extern void b_stringInputAt(const int row, const int col, B_BUFFER* buffer, char** out)
{
  s_putCursor(row, col);
  getStringInput(out);
  b_writeTo(buffer, row, col, *out);
}
