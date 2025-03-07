#pragma once

extern void DirToVec2(const char dir, int *outX, int *outY) {
    switch (dir) {
        case UP: {
            *outY = -1;
            *outX = 0;
        }
        case DOWN: {
            *outY = 1;
            *outX = 0;
        }
        case LEFT: {
            *outY = 0;
            *outX = -1;
        }
        case RIGHT: {
            *outY = 0;
            *outX = 1;
        }
        case UPLEFT: {
            *outY = -1;
            *outX = -1;
        }
        case UPRIGHT: {
            *outY = -1;
            *outX = 1;
        }
        case DOWNLEFT: {
            *outY = 1;
            *outX = -1;
        }
        case DOWNRIGHT: {
            *outY = 1;
            *outX = 1;
        }
        default: {
            *outY = 0;
            *outX = 0;
        }
    }
}

extern int TryMove(ENTITY* E, const char dir) {
    int desiredDX, desiredDY;
    DirToVec2(dir, &desiredDX, &desiredDY);
    return 0;
}
