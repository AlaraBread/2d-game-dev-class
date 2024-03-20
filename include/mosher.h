#include "physics.h"
#include "map.h"

#ifndef __ALARA_MOSHER__
#define __ALARA_MOSHER__

PhysicsBody *init_player_mosher(PhysicsWorld *world);
PhysicsBody *init_enemy_mosher(PhysicsWorld *world, EnemySpawn spawn);

#endif