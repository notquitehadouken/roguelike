#pragma once
#include <signal.h>
#ifdef USING_TERMIOS
#include <termios.h>
#endif

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
  signal(SIGINT, SIG_IGN);
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
}

enum INPUT
{
  FAIL = 0xFF,
  STR_COMM = '!',
  OPEN_HELP = '?',
  CONFIRM = '\n',
  EXAMINE = 'x',
#ifndef NUMPAD_USE
  UP_KEEP = 'K',
  DOWN_KEEP = 'J',
  LEFT_KEEP = 'H',
  RIGHT_KEEP = 'L',
  UPRIGHT_KEEP = 'U',
  UPLEFT_KEEP = 'Y',
  DOWNRIGHT_KEEP = 'N',
  DOWNLEFT_KEEP = 'B',
  HEIGHTUP_KEEP = 'I',
  HEIGHTDOWN_KEEP = 'M',
  UP = 'k',
  DOWN = 'j',
  LEFT = 'h',
  RIGHT = 'l',
  UPRIGHT = 'u',
  UPLEFT = 'y',
  DOWNRIGHT = 'n',
  DOWNLEFT = 'b',
  HEIGHTUP = 'i',
  HEIGHTDOWN = 'm',
  JUMP = ' ',
#else
  UP_KEEP = -1,
  DOWN_KEEP = -2,
  LEFT_KEEP = -3,
  RIGHT_KEEP = -4,
  UPRIGHT_KEEP = -5,
  UPLEFT_KEEP = -6,
  DOWNRIGHT_KEEP = -7,
  DOWNLEFT_KEEP = -8,
  HEIGHTUP_KEEP = -9,
  HEIGHTDOWN_KEEP = -10,
  UP = '8',
  DOWN = '2',
  LEFT = '4',
  RIGHT = '6',
  UPRIGHT = '9',
  UPLEFT = '7',
  DOWNRIGHT = '3',
  DOWNLEFT = '1',
  HEIGHTUP = '+',
  HEIGHTDOWN = '-',
  JUMP = '0',
#endif
  KEY_VIEWHEIGHT_UP = '>',
  KEY_VIEWHEIGHT_DOWN = '<',
  KEY_VIEWHEIGHT_C_RAISE = '.',
  KEY_VIEWHEIGHT_C_LOWER = ',',
  KEY_MAPCOLORSCALINGTOGGLE = ';',
  INVENTORY = 'e',
  SHORT_WAIT = 'w',
  LONG_WAIT = 'W',
  BUFFER_REDRAW = 0x100 | 'B',
  DEBUG_ENT = 0x100 | 'D',
  DEBUG_ENTS_ON_POS = 0x100 | 'P',
  I_CAST_EXPLODE_BALLS = 'E',
};

enum INPUT_SECONDARY
{
  SCREEN_SET
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
  case 0x1B: // Escape, so it must be arrow keys. (or a debug option)
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
    case 'P':
    case 'p':
      return setck(DEBUG_ENTS_ON_POS);
    case 'E':
    case 'e':
      return setck(I_CAST_EXPLODE_BALLS);
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