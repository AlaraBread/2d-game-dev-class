#include "gfc_list.h"
#include "gfc_vector.h"

#ifndef __ALARA_MAP__
#define __ALARA_MAP__

typedef enum EnemyType {
	NORMAL,
	SMALL,
	BIG,
} EnemyType;

typedef struct EnemySpawn {
	double spawn_time;
	EnemyType enemy_type;
	Vector2D spawn_position;
	Vector2D spawn_velocity;
} EnemySpawn;

List *map_get_nearby_secondary_beats(double time, double distance);
List *map_get_nearby_beats(double time, double distance);
void map_free();
void map_load(const char *filename);
List *map_get_nearby_secondary_beats(double time, double distance);
List *map_get_spawns(double time);

#endif