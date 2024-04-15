#include "gfc_types.h"
#include "gf2d_draw.h"
#include "rollback.h"
#include "ui_element.h"
#include "main_menu.h"
#include "mods.h"

void back_button_clicked(UIElement *back_button) {
	main_menu();
}

Bool g_mods_enabled[NUM_MODS] = {false};
char g_mod_names[NUM_MODS][64] = {"faster", "slower", "hoppy", "angry", "perfectionist"};

void mod_button_clicked(UIElement *mod_button) {
	g_mods_enabled[mod_button->index] = !g_mods_enabled[mod_button->index];
	if(mod_button->index == FASTER) {
		g_mods_enabled[SLOWER] = false;
	} else if (mod_button->index == SLOWER) {
		g_mods_enabled[FASTER] = false;
	}
}

void draw_mod_button(UIElement *mod_button) {
	draw_text_rect(mod_button);
	if(!g_mods_enabled[mod_button->index]) {
		return;
	}
	Vector2D a;
	vector2d_add(a, mod_button->position, mod_button->size);
	a.y -= mod_button->size.y/2;
	a.x -= 75;
	Vector2D b;
	b = a;
	b.x += 25;
	b.y += 25;
	Vector2D c;
	c = b;
	c.x += 25;
	c.y -= 75;
	Color color = gfc_color(1.0, 1.0, 1.0, 1.0);
	gf2d_draw_line(a, b, color);
	gf2d_draw_line(b, c, color);
}

extern Rollback g_rollback;
void mods() {
	PhysicsWorld *world = rollback_cur_physics(&g_rollback);
	physics_clear_bodies(world);
	clear_ui_elements();

	UIElement *back_button = create_button(vector2d(1200/2, 720-100), vector2d(300, 100), "back");
	back_button->click = back_button_clicked;

	for(int i = 0; i < NUM_MODS; i++) {
		UIElement *mod_button = create_button(vector2d(1200/2, 100+i*(75+20)), vector2d(400, 75), g_mod_names[i]);
		mod_button->font_size = 2;
		mod_button->draw = draw_mod_button;
		mod_button->index = i;
		mod_button->click = mod_button_clicked;
	}
}