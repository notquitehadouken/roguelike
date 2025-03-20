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

/**
 * Checks if two pixels are equal
 * @param a First pixel
 * @param b Second pixel
 * @return If they are equal
 */
extern char b_pixEq(const B_PIXEL *a, const B_PIXEL *b) {
    if (a == b) return 1;
    return (a->text == b->text && a->color == b->color);
}

/**
 * A screen buffer.
 */
struct BUFFER{
    char initialized;
    B_PIXEL *array[S_LENGTH];
};

typedef struct BUFFER B_BUFFER;

/**
 * Sets the initialized flag to true and forces every pixel in a buffer to be PIXEL_DEFAULT.
 * @param buffer The buffer
 */
extern void b_factory(B_BUFFER* buffer) {
    buffer->initialized = 1;
    for (int iter = 0; iter < S_LENGTH; iter++)
        buffer->array[iter] = &PIXEL_DEFAULT;
}

/**
 * Destroys a buffer
 * @param buffer The buffer
 */
extern void b_discard(B_BUFFER *buffer) {
    if (!buffer->initialized) return; // What?
    free(buffer);
}

/**
 * Initializes a buffer
 * @param buffer The buffer
 */
extern void b_initialize(B_BUFFER **buffer) {
    *buffer = (B_BUFFER*)malloc(sizeof(B_BUFFER));
    b_factory(*buffer);
}

/**
 * Returns the proper index for a buffer
 * @param row The row (Y value)
 * @param col The column (X value)
 * @return The index
 */
extern unsigned long b_getIndex(const int row, const int col) {    // so i don't need to write row * S_COL + col 5000 times
    return row * S_COL + col;
}

/**
 * Writes a pixel to a buffer
 * @param buffer The buffer
 * @param index The index
 * @param pixel The pixel
 */
extern void __WRITE(B_BUFFER *buffer, const int index, B_PIXEL *pixel) {
    if (!buffer->initialized)
        return;
    if (buffer->array[index] == pixel)
        return;
    buffer->array[index] = pixel;
}

/**
 * Gets a pixel from a buffer
 * @param buffer The buffer
 * @param row The row (Y value)
 * @param col The column (X value)
 * @param out The pixel
 */
extern void b_getPixel(const B_BUFFER *buffer, const int row, const int col, B_PIXEL **out) {
    *out = buffer->array[b_getIndex(row, col)];
}

/**
 * Sets a pixel to a buffer
 * @param buffer The buffer
 * @param row The row (Y value)
 * @param col The column (X value)
 * @param pixel The pixel
 */
extern void b_setPixel(B_BUFFER *buffer, const int row, const int col, B_PIXEL *pixel) {
    __WRITE(buffer, b_getIndex(row, col), pixel);
}

/**
 * Writes some colorful text to a buffer
 * @param buffer The buffer
 * @param row The row (Y value)
 * @param col The column (X value)
 * @param text The text
 * @param color The color
 */
extern void b_writeToColor(B_BUFFER *buffer, const int row, const int col, const char *text, char color) {
    const int index = b_getIndex(row, col);
    for (int i = 0; text[i]; i++) {
        B_PIXEL *P = malloc(2 * sizeof(char));
        P->text = text[i];
        P->color = color;
        __WRITE(buffer, index + i, P);
    }
}

/**
* Writes some default-color text to a buffer
 * @param buffer The buffer
 * @param row The row (Y value)
 * @param col The column (X value)
 * @param text The text
 */
extern void b_writeTo(B_BUFFER *buffer, const int row, const int col, const char *text) {
    b_writeToColor(buffer, row, col, text, B_DEFAULT_COLOR);
}

/**
 * Puts the cursor somewhere.
 * Used in printing
 * @param row The row (Y value)
 * @param col The column (X value)
 */
extern void s_putCursor(const int row, const int col) {
    fprintf(stdout, "\033[%i;%iH", row+1, col+1);
}

/**
 * Clears the entire screen.
 */
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

/**
 * Draws a buffer to the screen
 * @param buffer The buffer
 */
extern void b_draw(const B_BUFFER* buffer) {
    if (curbuffer == NULL || !curbuffer->initialized) {
        s_clearScreen();
        b_initialize(&curbuffer);
    }
    for (int row = 0; row < S_ROW; row++) {
        char *sBuffer = calloc(16 * S_COL, sizeof(char));
        char lastChanged = 0;
        int lastBufferEnd = 0;
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

/**
 * Forces a full redraw of a buffer
 * @param buffer The buffer
 */
extern void b_flush(const B_BUFFER *buffer) { // Triggers a full redraw of the screen
	if (curbuffer == NULL || !curbuffer->initialized) {
		b_initialize(&curbuffer);
	}
	b_factory(curbuffer);
	s_clearScreen();
	b_draw(buffer);
}

/**
 * Calculates every position in a map that the player can see
 * @param map The map
 * @param player The player entity
 * @return A MAP_LENGTH list of characters. The 2s place is if it is occluded, the 1s place is if it occludes.
 */
extern unsigned char *b_getOccluded(const ENTITY *map, const ENTITY *player) {
	unsigned char *occluded = calloc(MAP_LENGTH, sizeof(*occluded));
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
			occluded[x + y * S_COL] = 0b1;
	}
	int x, y;
	unsigned int *playerPos;
	GetDataFlag(player, FLAG_POS, (void**)&playerPos);
	ConvertToZXY(*playerPos, &x, &x, &y);
	for (int oX = 0; oX < MAP_WIDTH; oX++) {
		for (int oY = 0; oY < MAP_HEIGHT; oY++) {
			if (occluded[oX + oY * S_COL] & 0b10)
				continue;
			int count;
			int **line = createLine(oX - x, oY - y, &count);
			for (int i = 0; i < count - 1; i ++) {
				const int lX = line[i][0] + x;
				const int lY = line[i][1] + y;
				if (occluded[lX + lY * S_COL] & 0b1) {
					occluded[oX + oY * S_COL] |= 0b10;
					break;
				}
			}
			free(line);
		}
	}
	return occluded;
}

/**
 * Draws an entire map to the buffer.
 * @param buffer The buffer
 * @param map The map
 * @param player The player. Pass NULL to disable occlusion.
 */
extern void b_writeMapToBuffer(B_BUFFER *buffer, const ENTITY *map, const ENTITY *player) {
	unsigned char drawn[MAP_LENGTH];
	for (int i = 0; i < MAP_LENGTH; i++)
		drawn[i] = 0;
	ENTITY **ELIST;
	unsigned char *occluded = 0;
	if (player)
		occluded = b_getOccluded(map, player);
	GetDataFlag(map, FLAG_CONTAINER, (void**)&ELIST);
	for (int i = 0; ELIST[i]; i++) {
		const ENTITY *ent = ELIST[i];
		if (HasBoolFlag(ent, BFLAG_NORENDER))
			continue;
		int z, x, y;
		unsigned int *posDat;
		GetDataFlag(ent, FLAG_POS, (void**)&posDat);
		ConvertToZXY(*posDat, &z, &x, &y);
		if (player && occluded[x + y * S_COL] & 0b10) {
			b_setPixel(buffer, y + 2, x, &PIXEL_VISION_OBSCURED);
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

/**
 * Draws various snippets of information about the player to a buffer
 * @param buffer The buffer
 * @param player The player
 */
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