#pragma once
#if defined __unix__ || defined __APPLE__ && defined __MACH__
#define __USING_TERMIOS
#include <termios.h>
#else
#include <conio.h>
#endif
#define MAX_STR_LEN 64

#ifdef __USING_TERMIOS
void setUnbuffered() {
	struct termios settings;
	tcgetattr(0, &settings);
	settings.c_lflag &= ~ICANON;
	settings.c_cc[VTIME] = 0;
	settings.c_cc[VMIN] = 1;
	tcsetattr(0, &settings);
}
#endif

char __I;
char __getc(char* out) {
	char c = 0;
#ifdef __USING_TERMIOS
	setUnbuffered();
	c = getc(stdin);
#else
	c = _getche();
#endif
	*out = c;
	return *out;
}

extern char __setc(const char set) {
	__I = set;
	return set;
};

enum INPUT{
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
};

extern char qualifiesSolo(const char act) {
	switch(act) {
		case UP:
		case DOWN:
		case LEFT:
		case RIGHT:
		case UPRIGHT:
		case UPLEFT:
		case DOWNRIGHT:
		case DOWNLEFT:
		case UP_KEEP:
		case DOWN_KEEP:
		case LEFT_KEEP:
		case RIGHT_KEEP:
		case UPRIGHT_KEEP:
		case UPLEFT_KEEP:
		case DOWNRIGHT_KEEP:
		case DOWNLEFT_KEEP:
		case INVENTORY:
		case OPEN_HELP:
			return 1;
		default:
			return 0;
	}
}

extern char getNextInput() {
	__getc(&__I);
	switch(__I) {
		case 0x1B: // Escape, so it must be arrow keys.
			__getc(&__I); // Discard [
			__getc(&__I); // Read direction
			switch(__I) {
				case 'A':
					return __setc(UP);
				case 'B':
					return __setc(DOWN);
				case 'C':
					return __setc(RIGHT);
				case 'D':
					return __setc(LEFT);
				default:
					return __setc(FAIL); // What have you done?
			}
		default:
			return __I;
	}
}

extern void getStringInput(char **out) {
	char *str = malloc(MAX_STR_LEN * sizeof(char));
	char form[16];
	sprintf_s(form, sizeof(form), "%%%ds", MAX_STR_LEN - 1);
	scanf_s(form, str);
	*out = str;
}

extern char stringEqCaseless(const char *a, const char *b) {
	for (int i = 0; i < MAX_STR_LEN; i++) {
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

extern char stringEq(const char *a, const char *b) {
	for (int i = 0; i < MAX_STR_LEN; i++) {
		if (a[i] != b[i])
			return 0;
		if (a[i] == 0x0 && b[i] == 0x0)
			return 1;
	}
	return 1;
}