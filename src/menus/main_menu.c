#include <string.h>
#include "physics.h"
#include "rollback.h"
#include "ui_element.h"
#include "level_select.h"
#include "shop.h"
#include "mods.h"
#include "controls_menu.h"

void play_button_clicked(UIElement *play_button) {
	level_select();
}

void shop_button_clicked(UIElement *shop_button) {
	shop();
}

void mods_button_clicked(UIElement *mods_button) {
	mods();
}

void controls_button_clicked(UIElement *controls_button) {
	controls_menu();
}

extern Rollback g_rollback;
extern UIElement *g_focus;

void main_menu() {
	PhysicsWorld *world = rollback_cur_physics(&g_rollback);
	physics_clear_bodies(world);
	clear_ui_elements();
	UIElement *title_label = create_label(vector2d(1200/2, 100), CENTER, CENTER);
	strcpy(title_label->text, "moshing simulator");
	title_label->font_size = 10;
	UIElement *play = create_button(vector2d(1200/2, 720/2-100), vector2d(300, 100), "play");
	play->click = play_button_clicked;
	UIElement *shop = create_button(vector2d(1200/2, 720/2+20), vector2d(300, 100), "shop");
	shop->click = shop_button_clicked;
	UIElement *mods = create_button(vector2d(1200/2, 720/2-100+(100+20)*2), vector2d(300, 100), "mods");
	mods->click = mods_button_clicked;
	UIElement *controls = create_button(vector2d(1200/2, 720/2-100+(100+20)*3), vector2d(300, 100), "controls");
	controls->click = controls_button_clicked;

	play->next = shop;
	play->prev = controls;
	shop->next = mods;
	shop->prev = play;
	mods->next = controls;
	mods->prev = shop;
	controls->next = play;
	controls->prev = mods;

	g_focus = play;
}