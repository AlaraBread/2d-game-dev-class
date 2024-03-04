#include <string.h>
#include "physics.h"
#include "rollback.h"
#include "entity.h"

void play_button_clicked(Entity *play_button) {
	dance_floor();
}

void shop_button_clicked(Entity *shop_button) {
	shop();
}

extern Rollback g_rollback;

void main_menu() {
	PhysicsWorld *world = rollback_cur_physics(&g_rollback);
	physics_clear_bodies(world);
	clear_entities();
	Entity *play_button = create_button(vector2d(1200/2, 720/2), vector2d(300, 100), "Play");
	play_button->click = play_button_clicked;
	Entity *shop_button = create_button(vector2d(1200/2, 720/2+100+20), vector2d(300, 100), "Shop");
	shop_button->click = shop_button_clicked;
}