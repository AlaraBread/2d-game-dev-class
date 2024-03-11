#include <SDL.h>
#include "rollback.h"
#include "physics.h"
#include "wall.h"
#include "mosher.h"
#include "audio.h"
#include "map.h"
#include "entity.h"
#include "audio.h"
#include "gf2d_draw.h"
#include "simple_json_list.h"
#include "simple_json.h"
#include "simple_json_object.h"
#include "points.h"
#include "pause.h"

extern Rollback g_rollback;

void spawn_enemy(void *data) {
	EnemySpawn *enemy_spawn = (EnemySpawn *)data;
	if(!enemy_spawn) {
		return;
	}
	PhysicsWorld *world = rollback_cur_physics(&g_rollback);
	PhysicsBody *enemy = init_enemy_mosher(world);
	enemy->position = enemy_spawn->spawn_position;
	enemy->linear_velocity = enemy_spawn->spawn_velocity;
	// TODO: do something with enemy type
}

extern int g_mouse_x;
extern int g_mouse_y;

double beat_warning = 1.0;
void draw_beat(void *data) {
	double beat_time = *((double *)data);
	Vector2D center = vector2d(g_mouse_x, g_mouse_y);
	double delta = beat_time - get_beat_time();
	center.x -= delta * 100.0;
	Color color = gfc_color(0.0, 1.0, 0.0, 1.0-fabs(delta)/beat_warning);
	gf2d_draw_circle(center, 10, color);
}

double secondary_beat_warning = 2.0;
void draw_secondary_beat(void *data) {
	double beat_time = *((double *)data);
	Vector2D center = vector2d(g_mouse_x, g_mouse_y);
	double delta = beat_time - get_beat_time();
	center.y -= delta * 100.0;
	Color color = gfc_color(0.0, 1.0, 1.0, 1.0-fabs(delta)/secondary_beat_warning);
	gf2d_draw_circle(center, 10, color);
}

extern const Uint8 *g_keys;
extern Uint8 g_prev_keys[SDL_NUM_SCANCODES];
extern Bool g_paused;

List *g_nearby_beats;
List *g_nearby_secondary_beats;
void dance_floor_think(Entity *ent) {
	double beat_pos = get_beat_time();
	List *spawns = map_get_spawns(beat_pos);
	gfc_list_foreach(spawns, spawn_enemy);

	g_nearby_beats = map_get_nearby_beats(beat_pos, beat_warning);
	gfc_list_foreach(g_nearby_beats, draw_beat);

	g_nearby_secondary_beats = map_get_nearby_secondary_beats(beat_pos, secondary_beat_warning);
	gfc_list_foreach(g_nearby_secondary_beats, draw_secondary_beat);

	if((g_keys[SDL_SCANCODE_P] && !g_prev_keys[SDL_SCANCODE_P]) ||
			(g_keys[SDL_SCANCODE_ESCAPE] && !g_prev_keys[SDL_SCANCODE_ESCAPE])) {
		toggle_paused();
	}
}

extern int g_level_points;
void dance_floor_cleanup(Entity *dance_floor) {
	SJson *file = sj_load(dance_floor->filename);
	int stored_high_score;
	if(!sj_get_integer_value(sj_object_get_value(file, "high_score"), &stored_high_score)) {
		stored_high_score = -1;
	}
	if(stored_high_score < g_level_points) {
		sj_object_delete_key(file, "high_score");
		sj_object_insert(file, "high_score", sj_new_int(g_level_points));
		sj_save(file, dance_floor->filename);
	}
	sj_free(file);
}

Entity *create_dance_floor_entity() {
	Entity *ent = allocate_entity();
	ent->think = dance_floor_think;
	ent->cleanup = dance_floor_cleanup;
	return ent;
}

extern int g_level_points;
long int g_prev_points = -1;

void dance_floor(char *map_filename) {
	PhysicsWorld *world = rollback_cur_physics(&g_rollback);
	physics_clear_bodies(world);
	clear_entities();
	Entity *dance_floor_entity = create_dance_floor_entity();
	strcpy(dance_floor_entity->filename, map_filename);
	g_level_points = 0;
	create_points_label();

	float f = 1.0;
	float b = 0.8;

	init_floor(world, vector2d(500.0, 720.0-50.0), vector2d(0.0, -1.0), f, b);
	init_wall(world, vector2d(50.0, 700.0), vector2d(1.0, 0.0), f, b);
	init_wall(world, vector2d(1200.0-50.0, 700.0), vector2d(-1.0, 0.0), f, b);

	init_player_mosher(world);

	map_load(map_filename);
	play_music();
}