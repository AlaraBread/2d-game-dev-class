#include "rollback.h"
#include "physics.h"
#include "wall.h"
#include "mosher.h"
#include "audio.h"

extern Rollback g_rollback;

void dance_floor() {
	PhysicsWorld *world = rollback_cur_physics(&g_rollback);
	physics_clear_bodies(world);
	clear_entities();

	float f = 1.0;
	float b = 0.8;

	init_floor(world, vector2d(500.0, 720.0-50.0), vector2d(0.0, -1.0), f, b);
	init_wall(world, vector2d(50.0, 700.0), vector2d(1.0, 0.0), f, b);
	init_wall(world, vector2d(1200.0-50.0, 700.0), vector2d(-1.0, 0.0), f, b);

	for(int i = 0; i < 2; i++) {
		init_enemy_mosher(world);
	}
	init_player_mosher(world);

	play_music();
	set_bpm(160.0);
}