#include <string.h>
#include "physics.h"
#include "rollback.h"
#include "ui_element.h"
#include "level_select.h"
#include "shop.h"
#include "mods.h"

void play_button_clicked(UIElement *play_button) {
	level_select();
}

void shop_button_clicked(UIElement *shop_button) {
	shop();
}

void mods_button_clicked(UIElement *mods_button) {
	mods();
}

extern Rollback g_rollback;

void main_menu() {
	PhysicsWorld *world = rollback_cur_physics(&g_rollback);
	physics_clear_bodies(world);
	clear_entities();
	UIElement *title_label = create_label(vector2d(1200/2, 100), CENTER, CENTER);
	strcpy(title_label->text, "moshing simulator");
	title_label->font_size = 10;
	UIElement *play_button = create_button(vector2d(1200/2, 720/2), vector2d(300, 100), "play");
	play_button->click = play_button_clicked;
	UIElement *shop_button = create_button(vector2d(1200/2, 720/2+100+20), vector2d(300, 100), "shop");
	shop_button->click = shop_button_clicked;
	UIElement *mods_button = create_button(vector2d(1200/2, 720/2+(100+20)*2), vector2d(300, 100), "mods");
	mods_button->click = mods_button_clicked;
}