#include "gfc_types.h"
#include "ui_element.h"
#include "music.h"
#include "pause.h"
#include "main_menu.h"
#include "dance_floor.h"

void unpause_clicked(UIElement *unpause_button) {
	set_paused(false);
}

void main_menu_clicked(UIElement *main_menu_button) {
	set_paused(false);
	stop_music();
	main_menu();
}

extern char g_map_filename[512];
static void restart_clicked(UIElement *restart_button) {
	set_paused(false);
	dance_floor(g_map_filename);
}

extern UIElement *g_focus;
void create_pause_menu() {
	UIElement *tint = allocate_ui_element();
	tint->draw = draw_text_rect;
	tint->size = vector2d(1200, 720);
	tint->bg_color = gfc_color(0.0, 0.0, 0.0, 0.3);
	tint->group = UI_GROUP_PAUSE_MENU;

	UIElement *paused_label = create_label(vector2d(1200/2, 100), CENTER, CENTER);
	strcpy(paused_label->text, "paused");
	paused_label->font_size = 8;
	paused_label->group = UI_GROUP_PAUSE_MENU;

	UIElement *main_menu_button = create_button(vector2d(1200/2, 300), vector2d(500, 100), "main menu");
	main_menu_button->click = main_menu_clicked;
	main_menu_button->group = UI_GROUP_PAUSE_MENU;

	UIElement *restart_button = create_button(vector2d(1200/2, 300+120), vector2d(500, 100), "restart");
	restart_button->click = restart_clicked;
	restart_button->group = UI_GROUP_PAUSE_MENU;

	UIElement *unpause_button = create_button(vector2d(1200/2, 300+120*2), vector2d(500, 100), "unpause");
	unpause_button->click = unpause_clicked;
	unpause_button->group = UI_GROUP_PAUSE_MENU;

	main_menu_button->next = restart_button;
	main_menu_button->prev = unpause_button;
	restart_button->next = unpause_button;
	restart_button->prev = main_menu_button;
	unpause_button->next = main_menu_button;
	unpause_button->prev = restart_button;

	g_focus = unpause_button;
}

void remove_pause_menu() {
	clear_ui_group(UI_GROUP_PAUSE_MENU);
}

Bool g_paused = false;
extern int g_game_state;

void set_paused(Bool paused) {
	if(g_game_state != PLAYING) {
		paused = false;
	}
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