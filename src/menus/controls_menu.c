#include "gfc_list.h"
#include "rollback.h"
#include "rebind.h"
#include "main_menu.h"
#include "midi.h"

static void back_pressed(UIElement *element) {
	main_menu();
}

static void midi_pressed(UIElement *element) {
	midi_select();
}

extern Rollback g_rollback;
void controls_menu() {
	PhysicsWorld *world = rollback_cur_physics(&g_rollback);
	physics_clear_bodies(world);
	clear_ui_elements();
	List *buttons = gfc_list_new();
	for(int i = 0; i <= UI_ACCEPT; i++) {
		gfc_list_append(buttons, create_rebind_button(vector2d(1200/2, 100+(70)*i), vector2d(400, 50), i));
	}
	UIElement *back = create_button(vector2d(1200/2, 720-100), vector2d(300, 100), "back");
	back->click = back_pressed;
	UIElement *midi_button = create_button(vector2d(300/2, 720-50), vector2d(300, 100), "change midi input");
	midi_button->click = midi_pressed;
	midi_button->font_size = 2;

	back->next = midi_button;
	midi_button->prev = back;

	setup_list(buttons, midi_button, back);
	gfc_list_delete(buttons);
}