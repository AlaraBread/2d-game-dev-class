#include "gfc_types.h"
#include "entity.h"
#include "pause.h"
#include "dance_floor.h"
#include "main_menu.h"
#include "audio.h"

extern int g_game_state;

static void main_menu_clicked(Entity *main_menu_button) {
	stop_music();
	set_paused(false);
	main_menu();
}

void remove_end_screen();

extern char g_map_filename[512];
static void replay_clicked(Entity *replay_button) {
	remove_end_screen();
	set_paused(false);
	dance_floor(g_map_filename);
}

extern Rollback g_rollback;
void create_end_screen() {
	set_paused(false);

	PhysicsWorld *world = rollback_cur_physics(&g_rollback);

	Entity *tint = allocate_entity();
	tint->draw = draw_text_rect;
	tint->size = vector2d(1200, 720);
	tint->bg_color = gfc_color(0.0, 0.0, 0.0, 0.3);
	tint->group = UI_GROUP_END_MENU;

	Entity *game_over_label = create_label(vector2d(1200/2, 100), CENTER, CENTER);
	if(g_game_state == LOST) {
		strcpy(game_over_label->text, "you lost");
	} else {
		strcpy(game_over_label->text, "you win");
	}
	game_over_label->font_size = 8;
	game_over_label->group = UI_GROUP_END_MENU;

	Entity *replay_button = create_button(vector2d(1200/2, 720-100/2-20-120), vector2d(500, 100), "replay");
	replay_button->click = replay_clicked;
	replay_button->group = UI_GROUP_END_MENU;

	Entity *main_menu_button = create_button(vector2d(1200/2, 720-100/2-20), vector2d(500, 100), "main menu");
	main_menu_button->click = main_menu_clicked;
	main_menu_button->group = UI_GROUP_END_MENU;
}

void remove_end_screen() {
	clear_ui_group(UI_GROUP_END_MENU);
}