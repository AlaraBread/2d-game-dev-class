#include "physics.h"

#ifndef __ALARA_WALL__
#define __ALARA_WALL__

PhysicsBody *init_wall(PhysicsWorld *world, Vector2D position, Vector2D normal, float friction, float bounce);
PhysicsBody *init_floor(PhysicsWorld *world, Vector2D position, Vector2D normal, float friction, float bounce);

#endif