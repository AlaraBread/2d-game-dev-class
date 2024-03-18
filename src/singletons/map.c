#include <stdlib.h>
#include "simple_json.h"
#include "simple_json_array.h"
#include "simple_logger.h"
#include "gfc_vector.h"
#include "gfc_list.h"
#include "map.h"
#include "audio.h"

SJson *g_map = NULL;
double *g_secondary_beats = NULL;
unsigned int g_secondary_beats_len = 0;
double *g_beats = NULL;
unsigned int g_beats_len = 0;
double g_jump_velocity = 0.0;
const char *g_song_filename = NULL;
int g_high_score = 0;
EnemySpawn *g_enemy_spawns;
unsigned int g_enemy_spawns_len;

int *g_used_beats = NULL;
int *g_used_secondary_beats = NULL;

extern double g_bpm;

int double_cmp (const void *a, const void *b) {
	int a_d = *((double *)a);
	int b_d = *((double *)b);
	if (a_d > b_d) return  1;
	if (a_d < b_d) return -1;
	return 0;
}

// we are treating all time values as musical time
void map_load(const char *filename) {
	map_free();
	g_map = sj_load(filename);
	{
		SJson *secondary_beats_obj = sj_object_get_value(g_map, "secondary_beats");
		if(!secondary_beats_obj) {
			slog("No seconary_beats specified");
			map_free();
			return;
		}
		if(!sj_is_array(secondary_beats_obj)) {
			slog("secondary_beats is not an array %s", secondary_beats_obj->get_string(secondary_beats_obj));
			map_free();
			return;
		}
		SJList *secondary_beats = secondary_beats_obj->v.array;
		unsigned int len = sj_list_get_count(secondary_beats);
		g_secondary_beats = calloc(len, sizeof(double));
		g_secondary_beats_len = len;
		g_used_secondary_beats = calloc(sizeof(int), len);
		if(!g_secondary_beats) {
			slog("Ran out of memory :(");
			map_free();
			return;
		}
		for(int i = 0; i < len; i++) {
			SJson *element = sj_list_get_nth(secondary_beats, i);
			float v;
			if(!sj_get_float_value(element, &v)) {
				slog("Invalid secondary beat value %s", element->get_string(element));
				map_free();
				return;
			}
			g_secondary_beats[i] = v;
		}
		qsort(g_secondary_beats, g_secondary_beats_len, sizeof(double), double_cmp);
	}
	{
		SJson *beats_obj = sj_object_get_value(g_map, "beats");
		if(!beats_obj) {
			slog("No beats specified");
			map_free();
			return;
		}
		if(!sj_is_array(beats_obj)) {
			slog("beats is not an array %s", beats_obj->get_string(beats_obj));
			map_free();
			return;
		}
		SJList *beats = beats_obj->v.array;
		unsigned int len = sj_list_get_count(beats);
		g_beats = calloc(len, sizeof(double));
		g_beats_len = len;
		g_used_beats = calloc(sizeof(int), len);
		if(!g_beats) {
			slog("Ran out of memory :(");
			map_free();
			return;
		}
		for(int i = 0; i < len; i++) {
			SJson *element = sj_list_get_nth(beats, i);
			float v;
			if(!sj_get_float_value(element, &v)) {
				slog("Invalid beat value %s", element->get_string(element));
				map_free();
				return;
			}
			g_beats[i] = v;
		}
		qsort(g_beats, g_beats_len, sizeof(double), double_cmp);
	}
	{
		SJson *enemy_spawns_obj = sj_object_get_value(g_map, "enemy_spawns");
		if(!enemy_spawns_obj) {
			slog("No enemy_spawns specified");
			map_free();
			return;
		}
		if(!sj_is_array(enemy_spawns_obj)) {
			slog("enemy_spawns is not an array %s", enemy_spawns_obj->get_string(enemy_spawns_obj));
			map_free();
			return;
		}
		SJList *enemy_spawns = enemy_spawns_obj->v.array;
		unsigned int len = sj_list_get_count(enemy_spawns);
		g_enemy_spawns = calloc(len, sizeof(EnemySpawn));
		g_enemy_spawns_len = len;
		if(!g_enemy_spawns) {
			slog("Ran out of memory :(");
			map_free();
			return;
		}
		for(int i = 0; i < len; i++) {
			SJson *element = sj_list_get_nth(enemy_spawns, i);
			if(!sj_is_object(element)) {
				slog("Invalid enemy spawn %s", element->get_string(element));
				map_free();
				return;
			}
			EnemySpawn spawn;
			{
				SJson *spawn_time = sj_object_get_value(element, "time");
				if(!spawn_time) {
					slog("Enemy spawn %s does not contain time", element->get_string(element));
					map_free();
					return;
				}
				float f;
				if(!sj_get_float_value(spawn_time, &f)) {
					slog("%s is an invalid time", spawn_time->get_string(spawn_time));
					map_free();
					return;
				}
				spawn.spawn_time = f;
			}
			{
				SJson *enemy_type = sj_object_get_value(element, "type");
				if(!enemy_type) {
					slog("Enemy spawn %s does not contain type", element->get_string(element));
					map_free();
					return;
				}
				int i;
				if(!sj_get_integer_value(enemy_type, &i)) {
					slog("%s is an invalid type", enemy_type->get_string(enemy_type));
					map_free();
					return;
				}
				spawn.enemy_type = i;
			}
			{
				SJson *spawn_position_x = sj_object_get_value(element, "position_x");
				if(!spawn_position_x) {
					slog("Enemy spawn %s does not contain position_x", element->get_string(element));
					map_free();
					return;
				}
				float f;
				if(!sj_get_float_value(spawn_position_x, &f)) {
					slog("%s is an invalid position_x", spawn_position_x->get_string(spawn_position_x));
					map_free();
					return;
				}
				spawn.spawn_position.x = f;
			}
			{
				SJson *spawn_position_y = sj_object_get_value(element, "position_y");
				if(!spawn_position_y) {
					slog("Enemy spawn %s does not contain position_y", element->get_string(element));
					map_free();
					return;
				}
				float f;
				if(!sj_get_float_value(spawn_position_y, &f)) {
					slog("%s is an invalid position_y", spawn_position_y->get_string(spawn_position_y));
					map_free();
					return;
				}
				spawn.spawn_position.y = f;
			}
			{
				SJson *spawn_velocity_x = sj_object_get_value(element, "velocity_x");
				if(!spawn_velocity_x) {
					slog("Enemy spawn %s does not contain velocity_x", element->get_string(element));
					map_free();
					return;
				}
				float f;
				if(!sj_get_float_value(spawn_velocity_x, &f)) {
					slog("%s is an invalid velocity_x", spawn_velocity_x->get_string(spawn_velocity_x));
					map_free();
					return;
				}
				spawn.spawn_velocity.x = f;
			}
			{
				SJson *spawn_velocity_y = sj_object_get_value(element, "velocity_y");
				if(!spawn_velocity_y) {
					slog("Enemy spawn %s does not contain velocity_y", element->get_string(element));
					map_free();
					return;
				}
				float f;
				if(!sj_get_float_value(spawn_velocity_y, &f)) {
					slog("%s is an invalid velocity_y", spawn_velocity_y->get_string(spawn_velocity_y));
					map_free();
					return;
				}
				spawn.spawn_velocity.y = f;
			}
			g_enemy_spawns[i] = spawn;
		}
	}
	{
		SJson *jump_velocity = sj_object_get_value(g_map, "jump_velocity");
		if(!jump_velocity) {
			slog("No jump_velocity specified");
			map_free();
			return;
		}
		float jump_velocity_f;
		if(!sj_get_float_value(jump_velocity, &jump_velocity_f)) {
			slog("Invalid jump_velocity %s", jump_velocity->get_string(jump_velocity));
			map_free();
			return;
		}
		g_jump_velocity = jump_velocity_f;
	}
	{
		SJson *bpm = sj_object_get_value(g_map, "bpm");
		if(!bpm) {
			slog("No bpm specified");
			map_free();
			return;
		}
		float bpm_f;
		if(!sj_get_float_value(bpm, &bpm_f)) {
			slog("Invalid bpm %s", bpm->get_string(bpm));
			map_free();
			return;
		}
		set_bpm(bpm_f);
	}
	{
		SJson *song = sj_object_get_value(g_map, "song");
		if(!song) {
			slog("No song specified");
			map_free();
			return;
		}
		g_song_filename = sj_get_string_value(song);
		if(!g_song_filename) {
			slog("Invalid song %s", song->get_string(song));
			map_free();
			return;
		}
	}
	{
		SJson *high_score = sj_object_get_value(g_map, "high_score");
		if(!high_score) {
			g_high_score = 0;
			return;
		}
		if(!sj_get_integer_value(high_score, &g_high_score)) {
			g_high_score = 0;
			return;
		}
	}
}

Bool map_is_loaded() {
	return g_map != NULL;
}

void map_free() {
	if(g_map != NULL) {
		sj_free(g_map);
		g_map = NULL;
	}
	if(g_beats != NULL) {
		free(g_beats);
		g_beats = NULL;
	}
	if(g_secondary_beats != NULL) {
		free(g_secondary_beats);
		g_secondary_beats = NULL;
	}
	if(g_used_beats != NULL) {
		free(g_used_beats);
		g_used_beats = NULL;
	}
	if(g_used_secondary_beats != NULL) {
		free(g_used_secondary_beats);
		g_used_secondary_beats = NULL;
	}
	if(g_enemy_spawns != NULL) {
		free(g_enemy_spawns);
		g_enemy_spawns = NULL;
	}
	g_song_filename = NULL;
	set_bpm(0.0);
}

List *map_get_nearby_secondary_beats(double time, double distance) {
	if(!g_secondary_beats) {
		return NULL;
	}
	List *nearby = gfc_list_new();
	for(long int i = 0; i < g_secondary_beats_len; i++) {
		if(fabs(g_secondary_beats[i] - time) <= distance) {
			gfc_list_append(nearby, (void *)i);
		}
	}
	return nearby;
}

List *map_get_nearby_beats(double time, double distance) {
	if(!g_beats) {
		return NULL;
	}
	List *nearby = gfc_list_new();
	for(long int i = 0; i < g_beats_len; i++) {
		if(fabs(g_beats[i] - time) <= distance) {
			gfc_list_append(nearby, (void *)i);
		}
	}
	return nearby;
}

double g_prev_spawn_time = 0.0;
List *map_get_spawns(double time) {
	if(!g_enemy_spawns) {
		return NULL;
	}
	List *spawns = gfc_list_new();
	for(int i = 0; i < g_enemy_spawns_len; i++) {
		EnemySpawn *spawn = &g_enemy_spawns[i];
		if(spawn->spawn_time > g_prev_spawn_time && spawn->spawn_time < time) {
			gfc_list_append(spawns, spawn);
		}
	}
	g_prev_spawn_time = time;
	return spawns;
}