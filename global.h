/// <summary>
///     Various utility functions and enums that don't require game headers.
/// </summary>
#pragma once
#define MAX_STR_LEN 64

enum TIMEOF_ENUM
{
  TIMEOF_COLLIDE = 10,
  TIMEOF_SHORTWAIT = 25, // Time of pressing w (lowercase)
  TIMEOF_LONGWAIT = 100, // Time of pressing W (uppercase)
  TIMEOF_MOVE = 100, // How long it takes to move 1 tile by default
};

unsigned long long RANDOM_SEED = 0x8F52B09AA4510DC5;

/**
 * Scrambles RANDOM_SEED and produces a new value
 * @param variantize Any number. Used in randomization
 */
extern void random_scramble(long long variantize)
{
  RANDOM_SEED = ~RANDOM_SEED;
  RANDOM_SEED ^= variantize;
  RANDOM_SEED ^= RANDOM_SEED >> (sizeof(RANDOM_SEED) * 4 - 1);
  RANDOM_SEED ^= RANDOM_SEED << sizeof(RANDOM_SEED) * 3;
  RANDOM_SEED ^= (long long)&variantize << sizeof(RANDOM_SEED) * 2;
}

/**
 * Returns a random integer
 * @return A random integer
 */
extern int random_nextInt()
{
  random_scramble(0x1234fedc5678ba98);
  return RANDOM_SEED;
}

/**
 * Properly frees a line made using createLine
 */
extern void freeLine(int** line, const int count)
{
  int i;
  for (i = 0; i < count; i++)
    free(line[i]);
  free(line);
}

/**
 * Draws a line using a modified Bresenham's line algorithm
 * @param x The X position
 * @param y The Y position
 * @param z The Z position
 * @param wideswing Alternative mode
 * @param outCount How many points are in the return value
 * @return A list of offsets from 0, 0, 0 that draw a line
 */
extern attrmalloc int** createLine(int x, int y, int z, int wideswing, int* outCount)
{
  // Draws a line from 0, 0
  if (x == 0 && y == 0 && z == 0)
  {
    int** all = malloc(sizeof(int*));
    all[0] = calloc(3, sizeof(int));
    *outCount = 1;
    return all;
  }
  const char flipX = x < 0;
  const char flipY = y < 0;
  const char flipZ = z <= 0;
  x = flipX ? -x : x;
  y = flipY ? -y : y;
  z = flipZ ? -z : z;
  const char flipXY = x < y;
  if (flipXY)
  {
    const int t = x;
    x = y;
    y = t;
  }
  const char flipXZ = x < z;
  if (flipXZ)
  {
    const int t = x;
    x = z;
    z = t;
  }
  int** all = calloc(x + 1, sizeof(int*));
  int yD = 2 * y + (wideswing ? ((flipZ ^ flipXZ) ? (-2 * x + 1) : 0) : -x);
  int zD = 2 * z + (wideswing ? ((flipZ ^ flipXZ) ? (-2 * x + 1) : 0) : -x);
  int Y = 0;
  int Z = 0;
  int i;
  for (i = 0; i <= x; i++)
  {
    int* p = calloc(3, sizeof(int));
    all[i] = p;
    p[0] = i;
    p[1] = Y;
    p[2] = Z;
    if (yD > 0)
    {
      Y++;
      yD -= 2 * x;
    }
    yD += 2 * y;
    if (zD > 0)
    {
      Z++;
      zD -= 2 * x;
    }
    zD += 2 * z;
  }
  *outCount = x + 1;
  for (i = 0; i < *outCount; i++)
  {
    if (flipXZ)
    {
      const int t = all[i][0];
      all[i][0] = all[i][2];
      all[i][2] = t;
    }
    if (flipXY)
    {
      const int t = all[i][0];
      all[i][0] = all[i][1];
      all[i][1] = t;
    }
    if (flipX)
    {
      all[i][0] *= -1;
    }
    if (flipY)
    {
      all[i][1] *= -1;
    }
    if (flipZ)
    {
      all[i][2] *= -1;
    }
  }
  return all;
}

/**
 * Calculates an 8-bit color code for the given RGB
 * Accurate.
 * @param R Red value
 * @param G Green value
 * @param B Blue value
 * @return A proper color code
 */
extern attrconst unsigned char colorOf(const unsigned char R, const unsigned char G, const unsigned char B)
{
  if (R == G && G == B)
  {
    switch (R)
    {
    case 0: return 0;
    case 0xC0: return 7;
    case 0x80: return 8;
    case 0xFF: return 15;
    default: break;
    }
    // Greyscale values range
    // 2 3 2 to 2 5 5
    // So scale correctly
    if (R < 8)
      return 232;
    if (R > 238)
      return 255;
    return (R - 8) / 10 + 232;
  }
  if (R == G && B == 0)
  {
    switch (R)
    {
    default: break;
    case 0x80: return 3;
    case 0xFF: return 11;
    }
  }
  if (R == B && G == 0)
  {
    switch (R)
    {
    default: break;
    case 0x80: return 5;
    case 0xFF: return 13;
    }
  }
  if (B == G && R == 0)
  {
    switch (B)
    {
    default: break;
    case 0x80: return 6;
    case 0xFF: return 14;
    }
  }
  if (R == B && R == 0)
  {
    switch (G)
    {
    default: break;
    case 0x80: return 2;
    case 0xFF: return 10;
    }
  }
  if (R == G && R == 0)
  {
    switch (B)
    {
    default: break;
    case 0x80: return 4;
    case 0xFF: return 12;
    }
  }
  if (B == G && B == 0)
  {
    switch (R)
    {
    default: break;
    case 0x80: return 1;
    case 0xFF: return 9;
    }
  }
  // Glad that's over.
  char blueComponent = B >= 56 ? (B - 16) / 40 : 0;
  char greenComponent = G >= 56 ? (G - 16) / 40 : 0;
  char redComponent = R >= 56 ? (R - 16) / 40 : 0;
  return blueComponent + greenComponent * 6 + redComponent * 36 + 16;
}

extern void colorOf8Bit(const unsigned char color, unsigned char* R, unsigned char* G, unsigned char* B)
{
  if (color <= 15)
  {
    unsigned char intensity = color <= 8 ? 0x80 : 0xFF;
    if (color == 7)
      intensity = 0xc0;
    *R = (color % 2) ? intensity : 0;
    *G = (color % 4 >= 2) ? intensity : 0;
    *B = (color % 8 >= 4) ? intensity : 0;
    return;
  }
  if (color >= 232)
  {
    unsigned char intensity = (color - 232) * 10 + 8;
    if (intensity == 255)
      intensity = 0xee;
    *R = *G = *B = intensity;
    return;
  }
  unsigned char colorA = color - 16;
  char blueComponent = colorA % 6;
  char greenComponent = (colorA / 6) % 6;
  char redComponent = (colorA / 36) % 6;
  *B = blueComponent ? (blueComponent * 40 + 55) : 0;
  *G = greenComponent ? (greenComponent * 40 + 55) : 0;
  *R = redComponent ? (redComponent * 40 + 55) : 0;
}

extern attrmalloc uint* uintap(const uint val)
{
  uint* rValue = malloc(sizeof(uint));
  *rValue = val;
  return rValue;
}

extern attrmalloc int* intap(const int val)
{
  int* rValue = malloc(sizeof(int));
  *rValue = val;
  return rValue;
}

extern attrmalloc int* int2ap(const int val, const int val2)
{
  int* rValue = calloc(2, sizeof(int));
  rValue[0] = val;
  rValue[1] = val2;
  return rValue;
}

extern attrconst char inRange(const int val, const int low, const int high)
{
  return val >= low && val <= high;
}

extern attrconst int min(const int a, const int b)
{
  return (a < b) ? a : b;
}

extern attrconst int max(const int a, const int b)
{
  return (a > b) ? a : b;
}

extern attrconst int clamp(const int val, const int lower, const int higher)
{
  return (val < lower) ? lower : ((val > higher) ? higher : val);
}

extern attrconst char sign(const int val)
{
  return val ? ((val < 0) ? -1 : 1) : 0;
}

#define flip(n) (((n) <= 0) - (n))

/**
 * 0 -> 1 -> -1...
 */
extern void flipper(int *val)
{
  *val = flip(*val);
}

/**
 * Checks if two MAX_STR_LEN strings are equal.
 * Does not consider case
 * @param a The first string
 * @param b The second string
 * @return If they are equal
 */
extern attrpure char stringEqCaseless(const char* a, const char* b)
{
  int i;
  for (i = 0; i < MAX_STR_LEN; i++)
  {
    char tai = a[i];
    char tbi = b[i];
    if (tai >= 'A' && tai <= 'Z')
      tai += 'a' - 'A';
    if (tbi >= 'A' && tbi <= 'Z')
      tbi += 'a' - 'A';
    if (tai != tbi)
      return 0;
    if (tai == 0x0 && tbi == 0x0)
      return 1;
  }
  return 0;
}

/**
 * Checks if two MAX_STR_LEN strings are equal
 * @param a The first string
 * @param b The second string
 * @return If they are equal
 */
extern attrpure char stringEq(const char* a, const char* b)
{
  int i;
  for (i = 0; i < MAX_STR_LEN; i++)
  {
    if (a[i] != b[i])
      return 0;
    if (a[i] == 0x0 && b[i] == 0x0)
      return 1;
  }
  return 1;
}

/**
 * Fuck me.
 */

#define _CACHE_TYPE void
#define _CACHE_BLOCK_BITC 4 // 2^4 = 16
#define _CACHE_BLOCK_SIZE (0b1 << _CACHE_BLOCK_BITC)
#define _CACHE_BLOCK calloc(_CACHE_BLOCK_SIZE, sizeof(_CACHE_TYPE*))
#define _CACHE_OF(T) T***
#define _CACHE(N) (_CACHE_OF(_CACHE_TYPE))N
#define _CACHE_REF(N) (_CACHE_OF(_CACHE_TYPE)*)N
#define _CACHE_ACCESS(N, I) (N)[I >> _CACHE_BLOCK_BITC][I & (_CACHE_BLOCK_SIZE - 1)]

/**
 * Returns the amoutn of blocks in a cache.
 */
extern attrpure uint cacheBlockCount(_CACHE_OF(_CACHE_TYPE) Target)
{
  uint i = 0;
  while (Target[i++]);
  return i - 1;
}

/**
 * Returns the amount of elements in a cache.
 */
extern attrpure uint cacheLength(_CACHE_OF(_CACHE_TYPE) Target)
{
  uint blockCount = cacheBlockCount(Target);
  int i;
  for (i = blockCount * _CACHE_BLOCK_SIZE - 1; i >= 0; i--)
  {
    if (_CACHE_ACCESS(Target, i))
      return (uint)i + 1;
  }
  return 0;
}

/**
 * Returns whatever is at a given index in a cache.
 */
extern attrpure _CACHE_TYPE* cacheAccess(_CACHE_OF(_CACHE_TYPE) Target, const uint Ind)
{
  return _CACHE_ACCESS(Target, Ind);
}

/**
 * Returns a cache as a single, null-terminated memory block.
 */
extern attrmalloc _CACHE_TYPE** cacheAsList(_CACHE_OF(_CACHE_TYPE) Target)
{
  uint blockCount = cacheBlockCount(Target);
  uint listSize = blockCount * _CACHE_BLOCK_SIZE + 1;
  _CACHE_TYPE** list = calloc(listSize, sizeof(_CACHE_TYPE*));
  int i;
  for (i = 0; i < blockCount * _CACHE_BLOCK_SIZE; i++)
    list[i] = _CACHE_ACCESS(Target, i);
  return list;
}

/**
 * Initializes a cache with a length of 0
 */
extern void cacheInit(_CACHE_OF(_CACHE_TYPE)* Target)
{
  *Target = calloc(1, sizeof(_CACHE_TYPE**));
  (*Target)[0] = 0;
}

/**
 * Frees all blocks in a cache.
 * The pointer to the cache is maintained, and expected to be freed by the caller.
 */
extern void cacheDelete(_CACHE_OF(_CACHE_TYPE) Target)
{
  int i;
  for (i = 0; Target[i]; i++)
  {
    free(Target[i]); // yup that's the function
  }
}

/**
 * Adds another block to a cache.
 */
extern void cacheExpand(_CACHE_OF(_CACHE_TYPE)* Target)
{
  const uint blockCount = cacheBlockCount(*Target);
  // in a 1,1,0 array cap is 3
  _CACHE_TYPE*** NewReferenceArray = calloc(blockCount + 2, sizeof(_CACHE_TYPE**));
  int i;
  for (i = 0; i < blockCount; i++)
  {
    NewReferenceArray[i] = (*Target)[i];
  }
  free(*Target);
  *Target = NewReferenceArray;
  (*Target)[blockCount] = _CACHE_BLOCK;
}

/**
 * Adds an element to a cache.
 */
extern void cacheAdd(_CACHE_OF(_CACHE_TYPE)* Target, _CACHE_TYPE* Elem)
{
  int i;
  int blockCount = cacheBlockCount(*Target);
  for (i = 0; i < blockCount * _CACHE_BLOCK_SIZE; i++)
  {
    if (_CACHE_ACCESS(*Target, i) == Elem)
    {
      return;
    }
    if (!(_CACHE_ACCESS(*Target, i)))
    {
      _CACHE_ACCESS(*Target, i) = Elem;
      return;
    }
  }
  cacheExpand(Target);
  (*Target)[blockCount][0] = Elem;
}

/**
 * Removes an element from a cache.
 */
extern void cacheRemove(_CACHE_OF(_CACHE_TYPE) Target, const _CACHE_TYPE* Elem)
{
  int i = 0;
  int foundInd = -1;
  int elemCount = cacheBlockCount(Target) * _CACHE_BLOCK_SIZE;
  for (; i < elemCount; i++)
  {
    if (_CACHE_ACCESS(Target, i) == Elem)
    {
      foundInd = i;
      break;
    }
  }
  if (foundInd == -1)
    return;
  for (i = elemCount - 1; i >= 0; i--)
  {
    if (i == foundInd)
    {
      _CACHE_ACCESS(Target, foundInd) = 0;
      return;
    }
    if (_CACHE_ACCESS(Target, i))
    {
      _CACHE_ACCESS(Target, foundInd) = _CACHE_ACCESS(Target, i);
      _CACHE_ACCESS(Target, i) = 0;
      return;
    }
  }
}

/**
 * Minimizes the block count of a cache.
 */
extern void cacheShrink(_CACHE_OF(_CACHE_TYPE)* Target)
{
  uint blocks = cacheBlockCount(*Target);
  uint targetBlocks = blocks;
  int i;
  for (i = blocks - 1; i >= 0; i--)
  {
    if (Target[i][0]) break;
    free(Target[i]);
    targetBlocks -= 1;
  }
  _CACHE_TYPE*** NewArray = calloc(targetBlocks + 1, sizeof(_CACHE_TYPE*));
  for (i = 0; i < targetBlocks; i++)
  {
    NewArray[i] = (*Target)[i];
  }
  free(*Target);
  *Target = NewArray;
}
