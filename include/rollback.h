#include "physics.h"

#ifndef __ALARA_ROLLBACK__
#define __ALARA_ROLLBACK__

typedef struct Rollback {
	PhysicsWorld *buffer;
	int buffer_size;
	int buffer_index;
	int cutoff_index;
} Rollback;

Rollback init_rollback(int size, unsigned int max_physics_bodies);
void free_rollback(Rollback *rollback);
PhysicsWorld *rollback_step(Rollback *rollback, float delta);
PhysicsWorld *rollback_step_back(Rollback *rollback, float delta);

inline PhysicsWorld *rollback_next_physics(Rollback *rollback);
inline PhysicsWorld *rollback_cur_physics(Rollback *rollback);
inline PhysicsWorld *rollback_prev_physics(Rollback *rollback);

#endif