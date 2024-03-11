#include "gfc_types.h"
#include "entity.h"
#include "audio.h"
#include "pause.h"
#include "main_menu.h"

void unpause_clicked(Entity *unpause_button) {
	set_paused(false);
}

void main_menu_clicked(Entity *main_menu_button) {
	set_paused(false);
	stop_music();
	main_menu();
}

void create_pause_menu() {
	Entity *tint = allocate_entity();
	tint->draw = draw_text_rect;
	tint->size = vector2d(1200, 720);
	tint->bg_color = gfc_color(0.0, 0.0, 0.0, 0.3);
	tint->group = UI_GROUP_PAUSE_MENU;

	Entity *paused_label = create_label(vector2d(1200/2, 100), CENTER, CENTER);
	strcpy(paused_label->text, "Paused");
	paused_label->font_size = 8;
	paused_label->group = UI_GROUP_PAUSE_MENU;

	Entity *unpause_button = create_button(vector2d(1200/2, 420), vector2d(500, 100), "Unpause");
	unpause_button->click = unpause_clicked;
	unpause_button->group = UI_GROUP_PAUSE_MENU;

	Entity *main_menu_button = create_button(vector2d(1200/2, 300), vector2d(500, 100), "Main Menu");
	main_menu_button->click = main_menu_clicked;
	main_menu_button->group = UI_GROUP_PAUSE_MENU;
}

void remove_pause_menu() {
	clear_ui_group(UI_GROUP_PAUSE_MENU);
}

Bool g_paused = false;
void set_paused(Bool paused) {
	g_paused = paused;
	if(g_paused) {
		create_pause_menu();
		pause_music();
	} else {
		remove_pause_menu();
		resume_music();
	}
}

void toggle_paused() {
	set_paused(!g_paused);
}