#pragma once
#define MAX_STR_LEN 64

#ifdef USING_TERMIOS
void setUnbuffered() {
    struct termios settings;
    tcgetattr(0, &settings);
    settings.c_lflag &= ~ICANON;
    settings.c_cc[VTIME] = 0;
    settings.c_cc[VMIN] = 1;
    tcsetattr(0, TCSANOW, &settings);
}
#endif

int __I;
char __getc(int* out) {
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

extern int __setc(const int set) {
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
    SHORT_WAIT = 'w',
    LONG_WAIT = 'W',
    BUFFER_REDRAW = 0x10B,
};

extern int getNextInput() {
    __getc(&__I);
    switch(__I) {
        case 0x1B: // Escape, so it must be arrow keys.
            __getc(&__I);
            switch(__I) {
                case '[':
                    break;
            	case 'B':
				case 'b':
					return __setc(BUFFER_REDRAW);
			}
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
	sprintf(form,	"%%%ds", MAX_STR_LEN - 1);
	scanf(form, str);
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