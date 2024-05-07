#include "rollback.h"

Rollback init_rollback(int size, unsigned int max_physics_bodies) {
	Rollback rollback;
	rollback.buffer = calloc(sizeof(PhysicsWorld), size);
	rollback.buffer_size = size;
	rollback.cutoff_index = 1;
	rollback.buffer[0] = init_physics(max_physics_bodies, true);
	for(rollback.buffer_index = 0; rollback.buffer_index < size; rollback.buffer_index++) {
		rollback_cur_physics(&rollback)->prev = rollback_prev_physics(&rollback);
	}
	rollback.buffer_index = 0;
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
extern Uint32 g_mouse_buttons;
extern int g_mouse_x;
extern int g_mouse_y;
extern float g_midi_cc[129];
extern float g_midi_note[128];
PhysicsWorld *rollback_step(Rollback *rollback, float delta) {
	PhysicsWorld *cur = rollback_cur_physics(rollback);
	PhysicsWorld *next = rollback_next_physics(rollback);
	physics_copy(cur, next);
	memcpy(next->keys, g_keys, sizeof(Uint8)*SDL_NUM_SCANCODES);
	next->mouse_x = g_mouse_x;
	next->mouse_y = g_mouse_y;
	next->mouse_buttons = g_mouse_buttons;
	memcpy(next->midi_cc, g_midi_cc, sizeof(g_midi_cc));
	memcpy(next->midi_note, g_midi_note, sizeof(g_midi_note));
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