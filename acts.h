/// <summary>
///     This header contains TIMEOF data, which is the amount of time that certain actions take to complete
///     And also contains all the unwieldy functions that convert between certain types
///     And handles all entity-entity behavior
///     It is distinct from global.h in that it requires all other header files in order to function properly
///     As a consequence, no header files use acts.h
/// </summary>
#pragma once
#include "object.h"

enum TIMEOF_ENUM {
    TIMEOF_COLLIDE = 32, // The amount of time it takes to walk into something that does something.
    TIMEOF_SHORTWAIT = 32, // Time of pressing w (lowercase)
    TIMEOF_LONGWAIT = 256, // Time of pressing W (uppercase)
    TIMEOF_MOVEBASIC = 16, // How long it takes to move 1 tile by default
};

/**
 * Converts a direction constant to X and Y values
 * Up is considered (0, -1)
 * @param dir The direction
 * @param outX The X as output
 * @param outY The Y as output
 */
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

/**
 * Converts X and Y to a direction constant
 * @param x The X
 * @param y The Y
 * @param outD The direction constant as output
 */
extern void Vec2ToDir(const int x, const int y, char *outD) {
    if (x == 0 && y == 0) {
        *outD = 0;
        return;
    }
    if (x == 0 && y > 0) {
        *outD = DOWN;
        return;
    }
    if (x == 0 && y < 0) {
        *outD = UP;
        return;
    }

    if (x < 0 && y == 0) {
        *outD = LEFT;
        return;
    }
    if (x < 0 && y > 0) {
        *outD = DOWNLEFT;
        return;
    }
    if (x < 0 && y < 0) {
        *outD = UPLEFT;
        return;
    }

    if (x > 0 && y == 0) {
        *outD = RIGHT;
        return;
    }
    if (x > 0 && y > 0) {
        *outD = DOWNRIGHT;
        return;
    }
    if (x > 0 && y < 0) {
        *outD = UPRIGHT;
        return;
    }
}

/**
 * Changes the health of an entity cleanly
 * Positive means healing
 * @param E The entity
 * @param Change The change in health
 */
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

/**
 * The function called when something collides with something else
 * @param Collider The "colliding" entity
 * @param CollidedWith The "collided with" entity. This entity is not modified by the function
 * @return If anything happened
 */
extern char Collide(ENTITY *Collider, const ENTITY *CollidedWith) {
    return 0;
}

/**
 * The functioned called when something walks on (moves over) something else
 * @param Walker The entity walking
 * @param WalkedOver The entity being walked over
 * @return
 */
extern char OnWalkOver(ENTITY *Walker, ENTITY *WalkedOver) {
    if (HasDataFlag(WalkedOver, FLAG_CHANGE_HP_ON_STEP)) {
        int *HC;
        GetDataFlag(WalkedOver, FLAG_CHANGE_HP_ON_STEP, (void**)&HC);
        ChangeHealth(Walker, *HC);
    }
    return 0;
}

/**
 * Attempt to move an entity with this direction constant
 * @param E The entity to be moved
 * @param dir The direction constant
 * @return How long movement took. 0 means the movement failed.
 */
extern int TryMove(ENTITY* E, const int dir) {
    int desiredDX, desiredDY;
    DirToVec2(dir, &desiredDX, &desiredDY);
    if (desiredDX == 0 && desiredDY == 0)
        return 0;
    int _, x, y;
    unsigned int *posDat;
    ENTITY **mapRef;
    GetDataFlag(E, FLAG_CONTAINEDBY, (void**)&mapRef);
    if (!mapRef) // Something has gone wrong, but sure.
        return 0;
    const ENTITY *map = *mapRef;
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
                free(ELIST);
                return TIMEOF_COLLIDE;
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
    return TIMEOF_MOVEBASIC;
}

/**
 * Handles controller logic. Called every game loop for every entity with a controller.
 * @param controller The controller object
 * @param game The game object
 * @return If anything happened
 */
extern char ControllerProcess(ENTITY_CONTROLLER *controller, const ENTITY *game) {
    switch (controller->type) {
        default:
        case CONT_NOACTION:
        case CONT_DEAD: {
            controller->nextAct = GLOBAL_TIMER + 1;
            return 0;
        }
        case CONT_MOVETOPLAYER: {
            ENTITY *player;
            GetDataFlag(game, FLAG_PLAYER, (void**)&player);

            ENTITY *us = controller->associate;

            int pX, pY, uX, uY, *pP, *uP;
            GetDataFlag(player, FLAG_POS, (void**)&pP);
            GetDataFlag(us, FLAG_POS, (void**)&uP);
            if (!pP || !uP)
                break;

            ConvertToZXY(*pP, &pX, &pX, &pY);
            ConvertToZXY(*uP, &uX, &uX, &uY);

            char dir;
            Vec2ToDir(pX - uX, pY - uY, &dir);
            const int move = TryMove(us, dir);
            controller->nextAct += ScaleTime(move, us);
            if (move <= 0)
                controller->nextAct += ScaleTime(TIMEOF_LONGWAIT, us);
            return 1;
        }
    }
}