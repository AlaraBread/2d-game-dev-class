#include "rollback.h"
#include "entity.h"
#include "main_menu.h"

extern Rollback g_rollback;

void back_button_clicked(Entity *back_button) {
	main_menu();
}

int selected_powerup = -1;
void draw_powerup_button(Entity *button) {
	draw_text_rect(button);
	if(selected_powerup == button->index) {
		font_render_aligned("Selected -> ", 3, button->color, gfc_sdl_rect(button->position.x-button->size.x, button->position.y, button->size.x, button->size.y), END, CENTER);
	}
}

#define NUM_POWERUPS 5
char powerup_names[NUM_POWERUPS][64] = {"Grappling Hook", "Inflatable Shirt", "Strong Legs", "Free Hugs", "Drunk"};
int powerup_costs[NUM_POWERUPS] = {100, 200, 300, 400, 500};
Bool powerup_ownership[NUM_POWERUPS] = {false};
extern long int points;
Entity *points_label;
void update_powerup(Entity *powerup_button) {
	if(powerup_ownership[powerup_button->index]) {
		sprintf(powerup_button->text, " %s", powerup_names[powerup_button->index]);
	} else {
		sprintf(powerup_button->text, " %d - %s", powerup_costs[powerup_button->index], powerup_names[powerup_button->index]);
	}
}

void powerup_button_clicked(Entity *powerup_button) {
	if(!powerup_ownership[powerup_button->index]) {
		if(points < powerup_costs[powerup_button->index]) {
			return;
		}
		points -= powerup_costs[powerup_button->index];
		powerup_ownership[powerup_button->index] = true;
	}
	selected_powerup = powerup_button->index;
	update_powerup(powerup_button);

	TextLine points_text;
	sprintf(points_text, "Points: %d", points);
	strcpy(points_label->text, points_text);
}

void shop() {
	PhysicsWorld *world = rollback_cur_physics(&g_rollback);
	physics_clear_bodies(world);
	clear_entities();

	Entity *back_button = create_button(vector2d(1200/2, 720-100-20), vector2d(300, 100), "Back");
	back_button->click = back_button_clicked;

	for(int i = 0; i < 5; i++) {
		Entity *powerup_button = create_button(vector2d(1200/2, 100+i*(75+20)), vector2d(500, 75), powerup_names[i]);
		powerup_button->text_align_x = START;
		powerup_button->font_size = 2;
		powerup_button->draw = draw_powerup_button;
		powerup_button->index = i;
		powerup_button->click = powerup_button_clicked;
		update_powerup(powerup_button);
	}

	TextLine points_text;
	sprintf(points_text, "Points: %d", points);
	points_label = create_label(vector2d(0, 0), START, START, points_text);
}