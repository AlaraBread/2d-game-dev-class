#include "rollback.h"
#include "physics.h"
#include "wall.h"
#include "mosher.h"
#include "audio.h"
#include "map.h"
#include "entity.h"
#include "audio.h"
#include "gf2d_draw.h"

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
}

void create_dance_floor_entity() {
	Entity *ent = allocate_entity();
	ent->think = dance_floor_think;
}

void dance_floor() {
	PhysicsWorld *world = rollback_cur_physics(&g_rollback);
	physics_clear_bodies(world);
	clear_entities();
	create_dance_floor_entity();

	float f = 1.0;
	float b = 0.8;

	init_floor(world, vector2d(500.0, 720.0-50.0), vector2d(0.0, -1.0), f, b);
	init_wall(world, vector2d(50.0, 700.0), vector2d(1.0, 0.0), f, b);
	init_wall(world, vector2d(1200.0-50.0, 700.0), vector2d(-1.0, 0.0), f, b);

	init_player_mosher(world);

	map_load("sound/maps/laurasaidimblazed.json");

	play_music();
}