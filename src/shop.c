#include "rollback.h"
#include "entity.h"
#include "main_menu.h"
#include "shop.h"

extern Rollback g_rollback;

static void back_button_clicked(Entity *back_button) {
	main_menu();
}

extern int g_selected_powerup;
void draw_powerup_button(Entity *button) {
	draw_text_rect(button);
	if(g_selected_powerup == button->index) {
		font_render_aligned("Selected -> ", 3, button->color, gfc_sdl_rect(button->position.x-button->size.x, button->position.y, button->size.x, button->size.y), END, CENTER);
	}
}

char g_powerup_names[NUM_POWERUPS][64] = {"Grappling Hook", "Inflatable Shirt", "Strong Legs", "Free Hugs", "Drunk"};
int g_powerup_costs[NUM_POWERUPS] = {100, 200, 300, 400, 500};
extern Bool g_powerup_ownership[NUM_POWERUPS];
extern int g_points;
extern int g_level_points;
static Entity *g_points_label;
void update_powerup(Entity *powerup_button) {
	if(g_powerup_ownership[powerup_button->index]) {
		sprintf(powerup_button->text, " %s", g_powerup_names[powerup_button->index]);
	} else {
		sprintf(powerup_button->text, " $%d - %s", g_powerup_costs[powerup_button->index], g_powerup_names[powerup_button->index]);
	}
}

void powerup_button_clicked(Entity *powerup_button) {
	if(!g_powerup_ownership[powerup_button->index]) {
		if(g_points < g_powerup_costs[powerup_button->index]) {
			return;
		}
		g_points -= g_powerup_costs[powerup_button->index];
		g_powerup_ownership[powerup_button->index] = true;
	}
	if(g_selected_powerup != powerup_button->index) {
		g_selected_powerup = powerup_button->index;
	} else {
		g_selected_powerup = -1;
	}
	update_powerup(powerup_button);
}

void shop() {
	PhysicsWorld *world = rollback_cur_physics(&g_rollback);
	physics_clear_bodies(world);
	clear_entities();
	g_level_points = -1;

	Entity *back_button = create_button(vector2d(1200/2, 720-100), vector2d(300, 100), "Back");
	back_button->click = back_button_clicked;

	for(int i = 0; i < 5; i++) {
		Entity *powerup_button = create_button(vector2d(1200/2, 100+i*(75+20)), vector2d(500, 75), g_powerup_names[i]);
		powerup_button->text_align_x = START;
		powerup_button->font_size = 2;
		powerup_button->draw = draw_powerup_button;
		powerup_button->index = i;
		powerup_button->click = powerup_button_clicked;
		update_powerup(powerup_button);
	}
	g_points_label = create_points_label();
}