#pragma once
#define MAX_STR_LEN 64

#ifdef USING_TERMIOS
void setUnbuffered()
{
  struct termios settings;
  tcgetattr(0, &settings);
  settings.c_lflag &= ~ICANON;
  settings.c_cc[VTIME] = 0;
  settings.c_cc[VMIN] = 1;
  tcsetattr(0, TCSANOW, &settings);
}
#endif

int l_I;

char getck(int* out)
{
  char c = 0;
#ifdef USING_TERMIOS
  setUnbuffered();
  c = getc(stdin);
#else
  c = _getche();
#endif
  *out = (int)c;
  return *out;
}

/**
 * Sets the current value of l_I
 * @param set The new value
 * @return The value of set
 */
int setck(const int set)
{
  l_I = set;
  return set;
};

enum INPUT
{
  FAIL = (char)0xFF,
  STR_COMM = '!',
  OPEN_HELP = '?',
#ifndef NUMPAD_USE
  UP_KEEP = 'K',
  DOWN_KEEP = 'J',
  LEFT_KEEP = 'H',
  RIGHT_KEEP = 'L',
  UPRIGHT_KEEP = 'U',
  UPLEFT_KEEP = 'Y',
  DOWNRIGHT_KEEP = 'N',
  DOWNLEFT_KEEP = 'B',
  UP = 'k',
  DOWN = 'j',
  LEFT = 'h',
  RIGHT = 'l',
  UPRIGHT = 'u',
  UPLEFT = 'y',
  DOWNRIGHT = 'n',
  DOWNLEFT = 'b',
#else
  UP_KEEP = 0,
  DOWN_KEEP = 0,
  LEFT_KEEP = 0,
  RIGHT_KEEP = 0,
  UPRIGHT_KEEP = 0,
  UPLEFT_KEEP = 0,
  DOWNRIGHT_KEEP = 0,
  DOWNLEFT_KEEP = 0,
  UP = '8',
  DOWN = '2',
  LEFT = '4',
  RIGHT = '6',
  UPRIGHT = '9',
  UPLEFT = '7',
  DOWNRIGHT = '3',
  DOWNLEFT = '1',
#endif
  INVENTORY = 'i',
  SHORT_WAIT = 'w',
  LONG_WAIT = 'W',
  BUFFER_REDRAW = 0x10B,
  DEBUG_ENT = 0x10D,
};

/**
 * Waits for player input and sets l_I to it
 * @return The inputted key
 */
extern int getNextInput()
{
  getck(&l_I);
  switch (l_I)
  {
  case 0x1B: // Escape, so it must be arrow keys.
    getck(&l_I);
    switch (l_I)
    {
    case '[':
      break;
    case 'B':
    case 'b':
      return setck(BUFFER_REDRAW);
    case 'D':
    case 'd':
      return setck(DEBUG_ENT);
    default:
      return setck(FAIL);
    }
    getck(&l_I); // Read direction
    switch (l_I)
    {
    case 'A':
      return setck(UP);
    case 'B':
      return setck(DOWN);
    case 'C':
      return setck(RIGHT);
    case 'D':
      return setck(LEFT);
    default:
      return setck(FAIL); // What have you done?
    }
  default:
    return l_I;
  }
}

/**
 * Gets input from the user as a string.
 * Maximum length is MAX_STR_LEN
 * @param out
 */
extern void getStringInput(char** out)
{
  char* str = malloc(MAX_STR_LEN * sizeof(char));
  char form[16];
  sprintf(form, "%%%ds", MAX_STR_LEN - 1);
  scanf(form, str);
  *out = str;
}

/**
 * Checks if two MAX_STR_LEN strings are equal.
 * Does not consider case
 * @param a The first string
 * @param b The second string
 * @return If they are equal
 */
extern char stringEqCaseless(const char* a, const char* b)
{
  for (int i = 0; i < MAX_STR_LEN; i++)
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
extern char stringEq(const char* a, const char* b)
{
  for (int i = 0; i < MAX_STR_LEN; i++)
  {
    if (a[i] != b[i])
      return 0;
    if (a[i] == 0x0 && b[i] == 0x0)
      return 1;
  }
  return 1;
}
