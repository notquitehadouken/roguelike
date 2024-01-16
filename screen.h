#define SCREEN_WIDTH 50
#define SCREEN_HEIGHT 20

typedef struct {
    char color;
    char text;
} s_pixel;

typedef struct {
    s_pixel array[SCREEN_HEIGHT][SCREEN_WIDTH];
    bool initialized;
} s_array;

extern s_array createScreen(void) {
    s_array array;
    for (int iterY = 0; iterY < SCREEN_HEIGHT; iterY++) {
        for (int iterX = 0; iterX < SCREEN_WIDTH; iterX++) {
            array.array[iterY][iterX].color = 1;
            array.array[iterY][iterX].text = 'A';
        }
    }
    array.initialized = true;
    return array;
}

extern void display(s_array array) {
    puts("\033[3J\033[2J");
    for (int iterY = 0; iterY < SCREEN_HEIGHT; iterY++) {
        for (int iterX = 0; iterX < SCREEN_WIDTH; iterX++) {
            s_pixel pixel = array.array[iterY][iterX];
            printf("\033[%i;%iH\033[%cm%c", iterY+1, iterX+1, pixel.color, pixel.text);
        }
    }
}

extern void write(const char* restrict out){
    unsigned long len = 0;
    while(out[len++]);
}