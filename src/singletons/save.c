#include "gfc_types.h"
#include "simple_json.h"
#include "shop.h"

extern Bool g_powerup_ownership[NUM_POWERUPS];
extern int g_selected_powerup;
extern int g_points;

void init_save() {
	SJson *file = sj_load("save.json");
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

void save() {
	SJson *file = sj_object_new();
	sj_object_insert(file, "points", sj_new_int(g_points));
	SJson *powerup_ownership_json = sj_array_new();
	for(int i = 0; i < NUM_POWERUPS; i++) {
		SJson *val = sj_new_bool(g_powerup_ownership[i]);
		sj_array_append(powerup_ownership_json, val);
	}
	sj_object_insert(file, "powerup_ownership", powerup_ownership_json);
	sj_object_insert(file, "selected_powerup", sj_new_int(g_selected_powerup));
	sj_save(file, "save.json");
	sj_free(file); // this is a recursive free
}