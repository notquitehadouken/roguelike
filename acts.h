#pragma once

enum TIMEOF_ENUM {
    TIMEOF_COLLIDE = 32,
    TIMEOF_SHORTWAIT = 32,
    TIMEOF_LONGWAIT = 256,
};

extern void DirToVec2(const char dir, int *outX, int *outY) {
    switch (dir) {
        case UP_KEEP:
        case UP: {
            *outY = -1;
            *outX = 0;
            break;
        }
        case DOWN_KEEP:
        case DOWN: {
            *outY = 1;
            *outX = 0;
            break;
        }
        case LEFT_KEEP:
        case LEFT: {
            *outY = 0;
            *outX = -1;
            break;
        }
        case RIGHT_KEEP:
        case RIGHT: {
            *outY = 0;
            *outX = 1;
            break;
        }
        case UPLEFT_KEEP:
        case UPLEFT: {
            *outY = -1;
            *outX = -1;
            break;
        }
        case UPRIGHT_KEEP:
        case UPRIGHT: {
            *outY = -1;
            *outX = 1;
            break;
        }
        case DOWNLEFT_KEEP:
        case DOWNLEFT: {
            *outY = 1;
            *outX = -1;
            break;
        }
        case DOWNRIGHT_KEEP:
        case DOWNRIGHT: {
            *outY = 1;
            *outX = 1;
            break;
        }
        default: {
            *outY = 0;
            *outX = 0;
            break;
        }
    }
}

extern void ChangeHealth(ENTITY *E, const int Change) {
    int *HP;
    GetDataFlag(E, FLAG_HEALTH, (void**)&HP);
    if (!HP)
        return;
    HP[0] += Change;
    if (HP[0] > HP[1])
        HP[0] = HP[1];
    if (HP[0] < 0)
        HP[0] = 0;
}

extern char Collide(ENTITY *Collider, const ENTITY *CollidedWith) {
    return (long long)Collider % 2;
}

extern char OnWalkOver(ENTITY *Walker, ENTITY *WalkedOver) {
    if (HasDataFlag(WalkedOver, FLAG_CHANGE_HP_ON_STEP)) {
        int *HC;
        GetDataFlag(WalkedOver, FLAG_CHANGE_HP_ON_STEP, (void**)&HC);
        ChangeHealth(Walker, *HC);
    }
    return 0;
}

extern int TryMove(ENTITY* E, const int dir) {
    int desiredDX, desiredDY;
    DirToVec2(dir, &desiredDX, &desiredDY);
    if (desiredDX == 0 && desiredDY == 0)
        return 0;
    int _, x, y;
    unsigned int *posDat;
    ENTITY *map;
    GetDataFlag(E, FLAG_CONTAINEDBY, (void**)&map);
    if (!map) // Something has gone wrong, but sure.
        return 0;
    GetDataFlag(E, FLAG_POS, (void**)&posDat);
    ConvertToZXY(*posDat, &_, &x, &y);
    x += desiredDX;
    y += desiredDY;
    if (!InBounds(x, y))
        return 0;
    ENTITY **ELIST;
    int count = 0;
    if (!GetEntitiesOnPosition(map, x, y, &ELIST, &count))
        return 0;
    for (int i = 0; i < count; i++) {
        if (HasBoolFlag(ELIST[i], BFLAG_COLLIDABLE)) {
            if (Collide(E, ELIST[i]) || Collide(ELIST[i], E)) {
                GLOBAL_TIMER += TIMEOF_COLLIDE;
            }
            free(ELIST);
            return 0;
        }
    }
    for (int i = 0; i < count; i++) {
        OnWalkOver(E, ELIST[i]);
    }
    free(ELIST);
    ConvertToPosDat(_, x, y, posDat);
    GLOBAL_TIMER += 64;
    return 1;
}