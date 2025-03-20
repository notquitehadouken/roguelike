/// <summary>
///     Various utility functions that don't require game headers.
/// </summary>
#pragma once

unsigned long long RANDOM_SEED = 0x8F52B09AA4510DC5;

extern void random_scramble(long long variantize) {
    RANDOM_SEED = ~RANDOM_SEED;
    RANDOM_SEED ^= variantize;
    RANDOM_SEED ^= RANDOM_SEED >> sizeof(RANDOM_SEED) * 4 - 1;
    RANDOM_SEED ^= RANDOM_SEED << sizeof(RANDOM_SEED) * 3;
    RANDOM_SEED ^= (long long)&variantize << sizeof(RANDOM_SEED) * 2;
}

extern int random_nextInt() {
    random_scramble(0x1234fedc5678ba98);
    return RANDOM_SEED;
}

extern int **createLine(int x, int y, int *outCount) { // Draws a line from 0, 0
    if (x == 0 && y == 0) {
        int **all = malloc(sizeof(int*));
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
    if (flipXY) {
        const int z = x;
        x = y;
        y = z;
    }
    int **all = calloc(x + 1, sizeof(int*));
    int D = 2 * y - x;
    int Y = 0;
    for (int i = 0; i <= x; i++) {
        int *p = calloc(2, sizeof(int));
        all[i] = p;
        p[0] = i;
        p[1] = Y;
        if (D > 0) {
            Y++;
            D -= 2 * x;
        }
        D += 2 * y;
    }
    *outCount = x + 1;
    for (int i = 0; i < *outCount; i++) {
        if (flipXY) {
            const int z = all[i][0];
            all[i][0] = all[i][1];
            all[i][1] = z;
        }
        if (flipX) {
            all[i][0] *= -1;
        }
        if (flipY) {
            all[i][1] *= -1;
        }
    }
    return all;
}