#include <dirent.h>
#include "simple_json.h"
#include "simple_logger.h"
#include "rollback.h"
#include "ui_element.h"
#include "dance_floor.h"
#include "main_menu.h"

extern Rollback g_rollback;

static void back_button_clicked(UIElement *back_button) {
	main_menu();
}

char g_map_filename[512];
void map_button_clicked(UIElement *map_button) {
	strcpy(g_map_filename, map_button->filename);
	dance_floor(g_map_filename);
}

void draw_map_button(UIElement *map_button) {
	draw_text_rect(map_button);
	if(map_button->index != -1) {
		TextLine high_score;
		sprintf(high_score, "high score: $%d", map_button->index);
		font_render_aligned(high_score, 2, gfc_color(0.95, 0.97, 0.67, 1.0),
				gfc_sdl_rect(map_button->position.x+map_button->size.x+20,
				map_button->position.y, 0, map_button->size.y), START, CENTER);
	}
}

static int map_idx;
void list_map(char filename[256]) {
	char full_path[512] = "./sound/maps/";
	strcat(full_path, filename);
	SJson *file = sj_load(full_path);
	if(!file) {
		slog("Failed to list %s", full_path);
		return;
	}
	const char *title = sj_get_string_value(sj_object_get_value(file, "title"));
	int high_score;
	if(!sj_get_integer_value(sj_object_get_value(file, "high_score"), &high_score)) {
		high_score = -1;
	}

	UIElement *button = create_button(vector2d(1200/2-200, 100+map_idx*70), vector2d(600, 50), title);
	button->font_size = 2;
	button->click = map_button_clicked;
	strcpy(button->filename, full_path);
	button->index = high_score;
	button->draw = draw_map_button;

	sj_free(file);
	map_idx++;
}

void level_select() {
	PhysicsWorld *world = rollback_cur_physics(&g_rollback);
	physics_clear_bodies(world);
	clear_ui_elements();
	map_idx = 0;

	DIR *dp;
	struct dirent *ep;
	dp = opendir("./sound/maps");
	if(dp != NULL) {
		while ((ep = readdir (dp))) {
			if(strcmp("..", ep->d_name) == 0 || strcmp(".", ep->d_name) == 0) {
				continue;
			}
			list_map(ep->d_name);
		}
		(void) closedir(dp);
	} else {
		slog("Couldn't open map directory.");
	}

	UIElement *back_button = create_button(vector2d(1200/2, 720-100), vector2d(300, 100), "Back");
	back_button->click = back_button_clicked;
}