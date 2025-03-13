#pragma once
#define B_DEFAULT_TEXT ' '
#define B_DEFAULT_COLOR 39 // White

struct PIXEL{
    char text;
    char color;
};

typedef struct PIXEL B_PIXEL;

B_PIXEL PIXEL_DEFAULT = {B_DEFAULT_TEXT, B_DEFAULT_COLOR};
B_PIXEL PIXEL_VISION_OBSCURED = {'*', 90};

extern char b_pixEq(const B_PIXEL *a, const B_PIXEL *b) {
    if (a == b) return 1;
    return (a->text == b->text && a->color == b->color);
}

struct BUFFER{
    char initialized;
    B_PIXEL *array[S_LENGTH];
};

typedef struct BUFFER B_BUFFER;

extern void b_factory(B_BUFFER* buffer) {
    buffer->initialized = 1;
    for (int iter = 0; iter < S_LENGTH; iter++)
        buffer->array[iter] = &PIXEL_DEFAULT;
}

extern void b_discard(B_BUFFER *buffer) {
    if (!buffer->initialized) return; // What?
    free(buffer);
}

extern void b_initialize(B_BUFFER **buffer) {
    *buffer = (B_BUFFER*)malloc(sizeof(B_BUFFER));
    b_factory(*buffer);
}

extern unsigned long b_getIndex(const int row, const int col) {    // so i don't need to write row * S_COL + col 5000 times
    return row * S_COL + col;
}

extern void __WRITE(B_BUFFER *buffer, const int index, B_PIXEL *pixel) {
    if (!buffer->initialized)
        return;
    if (buffer->array[index] == pixel)
        return;
    buffer->array[index] = pixel;
}

extern void b_getPixel(const B_BUFFER *buffer, const int row, const int col, B_PIXEL **out) {
    *out = buffer->array[b_getIndex(row, col)];
}

extern void b_setPixel(B_BUFFER *buffer, const int row, const int col, B_PIXEL *pixel) {
    __WRITE(buffer, b_getIndex(row, col), pixel);
}

extern void b_writeToColor(B_BUFFER *buffer, const int row, const int col, const char *text, char color) {
    const int index = b_getIndex(row, col);
    for (int i = 0; text[i]; i++) {
        B_PIXEL *P = malloc(2 * sizeof(char));
        P->text = text[i];
        P->color = color;
        __WRITE(buffer, index + i, P);
    }
}

extern void b_writeTo(B_BUFFER *buffer, const int row, const int col, const char *text) {
    b_writeToColor(buffer, row, col, text, B_DEFAULT_COLOR);
}

extern void s_putCursor(const int row, const int col) {
    fprintf(stdout, "\033[%i;%iH", row+1, col+1);
}

extern void s_clearScreen() {
#ifdef __USING_WINDOWS
    HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode;
    GetConsoleMode(hOutput, &dwMode);
    dwMode |= ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOutput, dwMode);
#endif
    fputs("\033[3J\033[2J", stdout);
    fflush(stdout);
}

B_BUFFER *curbuffer = {0};

extern void b_draw(const B_BUFFER* buffer) {
    if (curbuffer == NULL || !curbuffer->initialized) {
        s_clearScreen();
        b_initialize(&curbuffer);
    }
    for (int row = 0; row < S_ROW; row++) {
        char *sBuffer = calloc(16 * S_COL, sizeof(char));
        char lastChanged = 0;
        char lastBufferEnd = 0;
        char lastColor = 0;
        int writeColumn = -1;
    	for (int col = 0; col < S_COL; col++) {
			B_PIXEL *pixel, *curpixel;
			b_getPixel(buffer, row, col, &pixel);
			b_getPixel(curbuffer, row, col, &curpixel);
			if (b_pixEq(pixel, curpixel)) {
				if (lastChanged) {
					s_putCursor(row, writeColumn);
					lastChanged = 0;
					writeColumn = -1;
					fputs(sBuffer + lastBufferEnd, stdout);
					lastBufferEnd = strlen(sBuffer);
				}
				continue;
			}
			lastChanged = 1;
			if (writeColumn == -1) {
				writeColumn = col;
			}
			if (lastColor != pixel->color) {
				lastColor = pixel->color;
				sprintf(sBuffer + strlen(sBuffer), "\033[0m\033[%dm", lastColor);
			}
			sprintf(sBuffer + strlen(sBuffer), "%c", pixel->text);
			b_setPixel(curbuffer, row, col, pixel);
		}
		if (lastChanged) {
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

extern void b_flush(const B_BUFFER *buffer) { // Triggers a full redraw of the screen
	if (curbuffer == NULL || !curbuffer->initialized) {
		b_initialize(&curbuffer);
	}
	b_factory(curbuffer);
	s_clearScreen();
	b_draw(buffer);
}

extern int *b_getOccluded(const ENTITY *map, const ENTITY *player) {
	int *occluded = calloc(MAP_LENGTH, sizeof(int));
	ENTITY **ELIST;
	GetDataFlag(map, FLAG_CONTAINER, (void**)&ELIST);
	for (int i = 0; i < CONTAINERCAPACITY; i++) {
		if (!ELIST[i])
			break;
		int x, y;
		unsigned int *pos;
		GetDataFlag(ELIST[i], FLAG_POS, (void**)&pos);
		ConvertToZXY(*pos, &x, &x, &y);
		if (HasBoolFlag(ELIST[i], BFLAG_OCCLUDING))
			occluded[x + y * S_COL] = 1;
	}
	int x, y;
	unsigned int *playerPos;
	GetDataFlag(player, FLAG_POS, (void**)&playerPos);
	ConvertToZXY(*playerPos, &x, &x, &y);
	for (int oX = 0; oX < MAP_WIDTH; oX++) {
		for (int oY = 0; oY < MAP_HEIGHT; oY++) {
			int count;
			int **line = createLine(oX - x, oY - y, &count);
			char occludedYet = 0;
			for (int i = 0; i < count; i ++) {
				const int lX = line[i][0] + x;
				const int lY = line[i][1] + y;
				if (occludedYet) {
					occluded[lX + lY * S_COL] = 2;
				}
				if (occluded[lX + lY * S_COL]) {
					occludedYet = 1;
				}
			}
		}
	}
	return occluded;
}

extern void b_writeMapToBuffer(B_BUFFER *buffer, const ENTITY *map, const ENTITY *player) {
	unsigned char drawn[MAP_LENGTH];
	for (int i = 0; i < MAP_LENGTH; i++)
		drawn[i] = 0;
	ENTITY **ELIST;
	int *occluded = b_getOccluded(map, player);
	GetDataFlag(map, FLAG_CONTAINER, (void**)&ELIST);
	for (int i = 0; ELIST[i]; i++) {
		int z, x, y;
		const ENTITY *ent = ELIST[i];
		unsigned int *posDat;
		GetDataFlag(ent, FLAG_POS, (void**)&posDat);
		ConvertToZXY(*posDat, &z, &x, &y);
		if (occluded[x + y * S_COL] == 2) {
			b_setPixel(buffer, y + 2, x, &PIXEL_DEFAULT);
			continue;
		}
		if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT)
			continue;
		const int index = x + y * S_COL;
		if (drawn[index] > z)
			continue;
		drawn[index] = z;
		B_PIXEL *pixel;
		GetDataFlag(ent, FLAG_APPEARANCE, (void**)&pixel);
		b_setPixel(buffer, y + 2, x, pixel);
	}
	free(occluded);
}

extern void b_writeHudToBuffer(B_BUFFER *buffer, const ENTITY *player) {
	int *HP;

	GetDataFlag(player, FLAG_HEALTH, (void**)&HP);

	if (HP) {
		char *HPText = calloc(64, sizeof(char));
		sprintf(HPText, "%d", HP[1]);
		int HPLen = strlen(HPText);
		free(HPText);
		HPText = calloc(HPLen * 2 + 4, sizeof(char));
		sprintf(HPText, "%d", HP[0]);
		for (int i = 0; i <= HPLen; i ++) {
			if (HPText[i] <= ' ')
				HPText[i] = ' ';
		}
		sprintf(HPText + HPLen, " / %d", HP[1]);

		char color = 37;
		const int ratio = HP[0] * 5 / HP[1];
		switch (ratio) {
			case 0: {
				color = 7;
				break;
			}
			case 1: {
				color = 31;
				break;
			}
			case 2: {
				color = 33;
				break;
			}
			case 3: {
				color = 32;
				break;
			}
			case 4: {
				color = 36;
				break;
			}
			default: {
				color = 37;
				break;
			}
		}
		b_writeToColor(buffer, S_ROW - 1, 1, HPText, color);

		free(HPText);
	}
}