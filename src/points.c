#include <stdlib.h>
#include "gfc_types.h"
#include "simple_json.h"
#include "entity.h"
#include "shop.h"

void save_points();

int g_points = 0;
int g_level_points = -1;
Bool g_powerup_ownership[NUM_POWERUPS] = {false};
int g_selected_powerup = -1;
void init_points() {
	SJson *file = sj_load("points.json");
	if(!file) {
		return;
	}
	sj_get_integer_value(sj_object_get_value(file, "points"), &g_points);
	SJson *powerup_ownership_json = sj_object_get_value(file, "powerup_ownership");
	if(sj_is_array(powerup_ownership_json)) {
		for(int i = 0; i < sj_array_get_count(powerup_ownership_json); i++) {
			SJson *value = sj_array_get_nth(powerup_ownership_json, i);
			sj_get_bool_value(value, &g_powerup_ownership[i]);
		}
	}
	sj_get_integer_value(sj_object_get_value(file, "selected_powerup"), &g_selected_powerup);
	sj_free(file);
}

void save_points() {
	SJson *file = sj_object_new();
	sj_object_insert(file, "points", sj_new_int(g_points));
	SJson *powerup_ownership_json = sj_array_new();
	for(int i = 0; i < NUM_POWERUPS; i++) {
		SJson *val = sj_new_bool(g_powerup_ownership[i]);
		sj_array_append(powerup_ownership_json, val);
	}
	sj_object_insert(file, "powerup_ownership", powerup_ownership_json);
	sj_object_insert(file, "selected_powerup", sj_new_int(g_selected_powerup));
	sj_save(file, "points.json");
	sj_free(file); // this is a recursive free
}

extern int g_high_score;
static void update_points_label(Entity *label) {
	int p = g_points;
	if(g_level_points != -1) {
		p = g_level_points;
		if(p > g_high_score) {
			label->color = gfc_color(0.95, 0.97, 0.67, 1.0);
		} else {
			label->color = gfc_color(1.0, 1.0, 1.0, 1.0);
		}
	}
	if(label->index != p) {
		sprintf(label->text, "$%d", p);
	}
	label->index = p;
}

Entity *create_points_label() {
	Entity *points_label = create_label(vector2d(0, 0), START, START);
	points_label->font_size = 4;
	points_label->index = -1;
	points_label->update = update_points_label;
	update_points_label(points_label);
	return points_label;
}