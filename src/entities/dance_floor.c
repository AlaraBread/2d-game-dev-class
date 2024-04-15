#include <SDL.h>
#include "rollback.h"
#include "physics.h"
#include "wall.h"
#include "mosher.h"
#include "audio.h"
#include "map.h"
#include "ui_element.h"
#include "audio.h"
#include "gf2d_draw.h"
#include "simple_json_list.h"
#include "simple_json.h"
#include "simple_json_object.h"
#include "points.h"
#include "pause.h"
#include "mods.h"
#include "dance_floor.h"

extern Rollback g_rollback;

void spawn_enemy(void *data) {
	EnemySpawn *enemy_spawn = (EnemySpawn *)data;
	if(!enemy_spawn) {
		return;
	}
	PhysicsWorld *world = rollback_cur_physics(&g_rollback);
	PhysicsBody *enemy = init_enemy_mosher(world, *enemy_spawn);
	if(enemy == NULL) {
		return;
	}
	enemy->position = enemy_spawn->spawn_position;
	enemy->linear_velocity = enemy_spawn->spawn_velocity;
}

extern int g_mouse_x;
extern int g_mouse_y;
UIElement *g_dance_floor;
extern double *g_beats;
extern double *g_secondary_beats;
extern int *g_used_beats;
extern int *g_used_secondary_beats;

double beat_warning = 3.0;
void draw_beat(void *data) {
	if(g_used_beats[(long int)data] != 0) {
		return;
	}
	double beat_time = g_beats[(long int)data];
	Vector2D center = g_dance_floor->position;
	double delta = beat_time - get_beat_time();
	center.x -= delta * 100.0;
	Color color = gfc_color(1.0, 1.0, 1.0, 1.0-fabs(delta)/beat_warning);
	gf2d_draw_circle(center, 10, color);
}

double secondary_beat_warning = 3.0;
void draw_secondary_beat(void *data) {
	if(g_used_secondary_beats[(long int)data] != 0) {
		return;
	}
	double beat_time = g_secondary_beats[(long int)data];
	Vector2D center = g_dance_floor->position;
	double delta = beat_time - get_beat_time();
	center.y -= delta * 100.0;
	Color color = gfc_color(1.0, 0.0, 1.0, 1.0-fabs(delta)/secondary_beat_warning);
	gf2d_draw_circle(center, 10, color);
}

extern double g_beat_interval;
extern double g_jump_velocity;
double get_beat_interval(double *beats, List *nearby_beats, double beat_time) {
	unsigned int len = gfc_list_get_count(nearby_beats);
	double prev_b = INFINITY;
	for(int i = 0; i < len; i++) {
		double b = beats[(long int)gfc_list_get_nth(nearby_beats, i)];
		if(!(b > beat_time && prev_b < beat_time)) {
			prev_b = b;
			continue;
		}
		return b-prev_b;
	}
	return INFINITY;
}

extern const Uint8 *g_keys;
extern Uint8 g_prev_keys[SDL_NUM_SCANCODES];
extern Bool g_paused;
extern int g_game_state;

List *g_nearby_beats;
List *g_nearby_secondary_beats;
void dance_floor_think(UIElement *element) {
	if(!g_paused && g_game_state == PLAYING) {
		element->position.x = g_mouse_x;
		element->position.y = g_mouse_y;
	}
	double beat_time = get_beat_time();
	List *spawns = map_get_spawns(beat_time);
	gfc_list_foreach(spawns, spawn_enemy);

	g_nearby_beats = map_get_nearby_beats(beat_time, beat_warning);
	g_nearby_secondary_beats = map_get_nearby_secondary_beats(beat_time, secondary_beat_warning);
	if(g_game_state == PLAYING) {
		gfc_list_foreach(g_nearby_beats, draw_beat);
		gfc_list_foreach(g_nearby_secondary_beats, draw_secondary_beat);
	}

	double interval = get_beat_interval(g_beats, g_nearby_beats, beat_time);
	PhysicsWorld *physics = rollback_cur_physics(&g_rollback);
	double speed = get_music_speed();
	if(interval == INFINITY) {
		physics->gravity = speed*speed*g_jump_velocity/g_beat_interval;
	} else {
		physics->gravity = speed*speed*2.0*g_jump_velocity/interval;
	}
	physics->jump_velocity = speed*g_jump_velocity;

	if((g_keys[SDL_SCANCODE_P] && !g_prev_keys[SDL_SCANCODE_P]) ||
			(g_keys[SDL_SCANCODE_ESCAPE] && !g_prev_keys[SDL_SCANCODE_ESCAPE])) {
		toggle_paused();
	}
}

extern int g_level_points;
void dance_floor_cleanup(UIElement *dance_floor) {
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

UIElement *create_dance_floor_element() {
	UIElement *element = allocate_entity();
	element->think = dance_floor_think;
	element->cleanup = dance_floor_cleanup;
	g_dance_floor = element;
	return element;
}

extern int g_level_points;
extern Bool g_mods_enabled[NUM_MODS];
long int g_prev_points = -1;
int g_game_state = PLAYING;

void dance_floor(char *map_filename) {
	PhysicsWorld *world = rollback_cur_physics(&g_rollback);
	physics_clear_bodies(world);
	clear_entities();

	g_game_state = PLAYING;

	UIElement *dance_floor = create_dance_floor_element();
	strcpy(dance_floor->filename, map_filename);
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

	if(g_mods_enabled[FASTER]) {
		set_music_speed(FASTER_SPEED);
	} else if(g_mods_enabled[SLOWER]) {
		set_music_speed(SLOWER_SPEED);
	} else {
		set_music_speed(1.0);
	}
}