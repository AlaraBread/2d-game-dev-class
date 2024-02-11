#include "physics.h"
#include "rollback.h"
#include "entity.h"

void play_button_clicked(Entity *play_button) {
	dance_floor();
}

extern Rollback g_rollback;

void main_menu() {
	PhysicsWorld *world = rollback_cur_physics(&g_rollback);
	physics_clear_bodies(world);
	clear_entities();
	Entity *play_button = allocate_entity();
	play_button->bg_color = gfc_color(0.1, 0.1, 0.1, 1.0);
	play_button->border_color = gfc_color(1.0, 1.0, 1.0, 1.0);
	play_button->color = gfc_color(1.0, 1.0, 1.0, 1.0);
	play_button->draw = draw_text_rect;
	play_button->mouse_enter = button_mouse_enter;
	play_button->mouse_exit = button_mouse_exit;
	play_button->mouse_down = button_mouse_down;
	play_button->mouse_up = button_mouse_up;
	play_button->click = play_button_clicked;
	play_button->position = vector2d(1200/2-300/2, 720/2-100/2);
	play_button->size = vector2d(300, 100);
	play_button->font_size = 5;
	play_button->text_align_x = CENTER;
	play_button->text_align_y = CENTER;
	
	char t[] = "Play!";
	memcpy(play_button->text, t, sizeof(t));
}