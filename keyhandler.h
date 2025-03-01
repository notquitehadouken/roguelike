#pragma once
#define NUMPAD_USE 0

char __I;

char __IRaw();

#ifdef __unix__
char __IRaw(char* out){
	if (system("stty raw")) // why is there no conio
		return 0x0; // system returns non-zero on fail
	char next = fgetc(stdin);
	if (system("stty -raw"))
		return 0x0;
	*out = next;
	return next;
};
#endif
#ifdef __CYGWIN__
char __IRaw(char* out){
	return getch(); // you're boned
};
#endif

inline char __ISet(const char set){
	__I = set;
	return set;
};

enum INPUT{
	FAIL = (char)0xFF,
	STR_COMM = '!',
	OPEN_HELP = '?',
#if NUMPAD_USE == 0
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

inline char qualifiesSolo(const char act){
	switch(act){
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

inline char getNextInput(){
	__IRaw(&__I);
	switch(__I){
		case 0x1B: // Escape, so it must be arrow keys.
			__IRaw(&__I); // Discard [
			__IRaw(&__I); // Read direction
			switch(__I){
				case 'A':
					return __ISet(UP);
				case 'B':
					return __ISet(DOWN);
				case 'C':
					return __ISet(RIGHT);
				case 'D':
					return __ISet(LEFT);
				default:
					return __ISet(FAIL); // What have you done?
			}
		default:
			return __I;
	}
}

inline int getStringInput(char *out){
	const char res = fgets(out, 64, stdin) ? 1 : 0;
	for(int i = 0; i < 64; i++){
		if (out[i] == '\n' || !out[i]){
			out[i] = 0;
			break;
		}
	}
	return res;
}

inline char stringEqCaseless(const char *a, const char *b){
	for (int i = 0; i < 64; i++){
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

inline char stringEq(const char *a, const char *b){
	for (int i = 0; i < 64; i++){
		if (a[i] != b[i])
			return 0;
		if (a[i] == 0x0 && b[i] == 0x0)
			return 1;
	}
	return 1;
}