/// <summary>
///     Various utility functions and enums that don't require game headers.
/// </summary>
#pragma once

enum TIMEOF_ENUM
{
  TIMEOF_COLLIDE = 32, // The amount of time it takes to walk into something that does something.
  TIMEOF_SHORTWAIT = 32, // Time of pressing w (lowercase)
  TIMEOF_LONGWAIT = 256, // Time of pressing W (uppercase)
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
 * Draws a line using Bresenham's line algorithm
 * @param x The X position
 * @param y The Y position
 * @param outCount How many points are in the return value
 * @return A list of offsets from 0, 0 that draw a line
 */
extern int** createLine(int x, int y, int* outCount)
{
  // Draws a line from 0, 0
  if (x == 0 && y == 0)
  {
    int** all = malloc(sizeof(int*));
    all[0] = calloc(2, sizeof(int));
    all[0][0] = 0;
    all[0][1] = 0;
    *outCount = 1;
    return all;
  }
  const char flipX = x < 0;
  const char flipY = y < 0;
  x = flipX ? -x : x;
  y = flipY ? -y : y;
  const char flipXY = x < y;
  if (flipXY)
  {
    const int z = x;
    x = y;
    y = z;
  }
  int** all = calloc(x + 1, sizeof(int*));
  int D = 2 * y - x;
  int Y = 0;
  for (int i = 0; i <= x; i++)
  {
    int* p = calloc(2, sizeof(int));
    all[i] = p;
    p[0] = i;
    p[1] = Y;
    if (D > 0)
    {
      Y++;
      D -= 2 * x;
    }
    D += 2 * y;
  }
  *outCount = x + 1;
  for (int i = 0; i < *outCount; i++)
  {
    if (flipXY)
    {
      const int z = all[i][0];
      all[i][0] = all[i][1];
      all[i][1] = z;
    }
    if (flipX)
    {
      all[i][0] *= -1;
    }
    if (flipY)
    {
      all[i][1] *= -1;
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
extern unsigned char colorOf(const unsigned char R, const unsigned char G, const unsigned char B)
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
    // on a comprehensible note the colors go 8,8,8 to 18,18,18 to 28,28,28 and onward
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
  // Why the fuck is it BGR? Nobody knows...
  return B / 48 + G / 48 * 6 + R / 48 * 36;
}
