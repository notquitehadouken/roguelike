#pragma once
#define S_ROW 50
#define S_COL 80
#define S_LENGTH S_ROW*S_COL
#define B_DEFAULT_TEXT ' '
#define B_DEFAULT_COLOR 39 // White
#include "gamestate.h"
#include "object.h"

struct __PIXEL{
	char text;
	char color;
};

typedef struct __PIXEL B_PIXEL;

B_PIXEL DEFAULT_PIXEL = {B_DEFAULT_TEXT, B_DEFAULT_COLOR};

extern char b_pixEq(const B_PIXEL *a, const B_PIXEL *b) {
	if (a == b) return 1;
	char bothInvis = (a->text <= ' ' || a->text >= 127) && (b->text <= ' ' || b->text >= 127);
	return bothInvis || (a->text == b->text && a->color == b->color);
}

struct __BUFFER{
	char initialized;
	B_PIXEL *array[S_LENGTH];
};

typedef struct __BUFFER B_BUFFER;

extern void b_factory(B_BUFFER* buffer) {
	buffer->initialized = 1;
	for (int iter = 0; iter < S_ROW*S_COL; iter++)
		buffer->array[iter] = &DEFAULT_PIXEL;
}

extern void b_discard(B_BUFFER *buffer) {
	if (!buffer->initialized) return; // What?
	free(buffer);
}

extern void b_initialize(B_BUFFER **buffer) {
	*buffer = (B_BUFFER*)malloc(sizeof(B_BUFFER));
	b_factory(*buffer);
}

extern unsigned long b_getIndex(int row, int col) {  // so i don't need to write row * S_COL + col 5000 times
	return row * S_COL + col;
}

extern void __WRITE(B_BUFFER *buffer, int index, B_PIXEL *pixel) {
	if (!buffer->initialized)
		return;
	if (buffer->array[index] == pixel)
		return;
	buffer->array[index] = pixel;
}

extern void b_getPixel(const B_BUFFER *buffer, int row, int col, B_PIXEL **out) {
	*out = buffer->array[b_getIndex(row, col)];
}

extern void b_setPixel(B_BUFFER *buffer, int row, int col, B_PIXEL *pixel) {
	__WRITE(buffer, b_getIndex(row, col), pixel);
}

extern void b_writeToColor(B_BUFFER *buffer, int row, int col, const char *text, char color) {
	const int index = b_getIndex(row, col);
	for (int i = 0; text[i]; i++) {
		B_PIXEL *P = malloc(2);
		P->text = text[i];
		P->color = color;
		__WRITE(buffer, index + i, P);
	}
}

extern void b_writeTo(B_BUFFER *buffer, int row, int col, const char *text) {
	b_writeToColor(buffer, row, col, text, B_DEFAULT_COLOR);
}

extern void s_putCursor(int row, int col) {
	printf("\033[%i;%iH", row+1, col+1);
}

extern void s_putPixel(const B_PIXEL *P) {
	printf("\033[%im%c\033[0m", P->color, P->text);
}

B_BUFFER *curbuffer = {0};

extern void b_draw(const B_BUFFER* buffer) {
	if (curbuffer == NULL || !curbuffer->initialized)
		b_initialize(&curbuffer);
	/* puts("\033[3J\033[2J"); /* deprecated until i find a better solution
	ED ansi code twice.
	3 means clear entire screen and buffer,
	2 means clear entire screen.
	Both are called because jank.*/
	for (int row = 0; row < S_ROW; row++) {
		for (int col = 0; col < S_COL; col++) {
			B_PIXEL *pixel, *curpixel;
			b_getPixel(buffer, row, col, &pixel);
			b_getPixel(curbuffer, row, col, &curpixel);
			if (b_pixEq(pixel, curpixel))
				continue;
			s_putCursor(row, col);
			s_putPixel(pixel);
			b_setPixel(curbuffer, row, col, pixel);
		}
	}
	s_putCursor(S_ROW, S_COL);
}

extern void b_writeMapToBuffer(B_BUFFER *buffer, const ENTITY *map) {
	unsigned char drawn[MAP_LENGTH];
	for (int i = 0; i < MAP_LENGTH; i++)
		drawn[i] = 0;

	ENTITY **ELIST;
	GetDataFlag(map, FLAG_CONTAINER, (void**)&ELIST);
	for (int i = 0; ELIST[i]; i++) {
		int z, x, y;
		const ENTITY *ent = ELIST[i];
		unsigned int *posDat;
		GetDataFlag(ent, FLAG_POS, (void**)&posDat);
		ConvertToZXY(*posDat, &z, &x, &y);
		if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT)
			continue;
		const int index = x + y * S_ROW;
		if (drawn[index] > z)
			continue;
		drawn[index] = z;
		B_PIXEL *pixel;
		GetDataFlag(ent, FLAG_APPEARANCE, (void**)&pixel);
		b_setPixel(buffer, y + 2, x, pixel);
	}
}