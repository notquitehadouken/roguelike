
#define MAP_WIDTH 80
#define MAP_HEIGHT 55

typedef struct {
    o_object objects[MAP_WIDTH][MAP_HEIGHT];
} game_map;

enum STATE {
    MENU,
    RUNNING,
    OVER,
  };

typedef struct {
    char state;
    game_map map;
    s_array screen;
} game_state;
