#include "rollback.h"

Rollback init_rollback(int size, unsigned int max_physics_bodies) {
	Rollback rollback;
	rollback.buffer = calloc(sizeof(PhysicsWorld), size);
	rollback.buffer_size = size;
	rollback.buffer_index = 0;
	rollback.cutoff_index = 1;
	rollback.buffer[0] = init_physics(max_physics_bodies, true);
	return rollback;
}

void free_rollback(Rollback *rollback) {
	for(int i = 0; i < rollback->buffer_size; i++) {
		free_physics(&rollback->buffer[i]);
	}
	free(rollback->buffer);
}

// https://stackoverflow.com/questions/14997165/fastest-way-to-get-a-positive-modulo-in-c-c
inline int posmod(int i, int n) {
    return (i % n + n) % n;
}

inline PhysicsWorld *rollback_next_physics(Rollback *rollback) {
	return &rollback->buffer[posmod(rollback->buffer_index+1, rollback->buffer_size)];
}

inline PhysicsWorld *rollback_cur_physics(Rollback *rollback) {
	return &rollback->buffer[rollback->buffer_index];
}

inline PhysicsWorld *rollback_prev_physics(Rollback *rollback) {
	return &rollback->buffer[posmod(rollback->buffer_index-1, rollback->buffer_size)];
}

extern const Uint8 *g_keys;
PhysicsWorld *rollback_step(Rollback *rollback, float delta) {
	PhysicsWorld *cur = rollback_cur_physics(rollback);
	PhysicsWorld *next = rollback_next_physics(rollback);
	physics_copy(cur, next);
	memcpy(next->keys, g_keys, sizeof(Uint8)*SDL_NUM_SCANCODES);
	physics_step(next, delta);
	rollback->buffer_index = posmod(rollback->buffer_index+1, rollback->buffer_size);
	rollback->cutoff_index = posmod(rollback->buffer_index+1, rollback->buffer_size);
	return next;
}

PhysicsWorld *rollback_step_back(Rollback *rollback, float delta) {
	if(rollback->cutoff_index != rollback->buffer_index && rollback_prev_physics(rollback)->physics_bodies != NULL) {
		rollback->buffer_index = posmod(rollback->buffer_index-1, rollback->buffer_size);
	}
	return rollback_cur_physics(rollback);
}