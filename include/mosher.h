#include "physics.h"

#ifndef __ALARA_MOSHER__
#define __ALARA_MOSHER__

PhysicsBody *init_player_mosher(PhysicsWorld *world);
PhysicsBody *init_enemy_mosher(PhysicsWorld *world);
void mosher_update(PhysicsBody *body, PhysicsWorld *world, float delta);

#endif