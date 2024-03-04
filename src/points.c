#include <stdlib.h>
#include "simple_json.h"

void save_points();

long int points = 0;
void init_points() {
	SJson *file = sj_load("points.json");
	if(!file) {
		return;
	}
	SJson *points_json = sj_object_get_value(file, "points");
	sj_get_integer_value(points_json, &points);
	sj_free(file);
	atexit(save_points);
}

void save_points() {
	SJson *file = sj_object_new();
	SJson *points_json = sj_new_int(points);
	sj_object_insert(file, "points", points_json);
	sj_save(file, "points.json");
	sj_free(file); // this is a recursive free
}