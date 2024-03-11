#include "gf2d_draw.h"
#include "entity.h"
#include "font.h"
#include "util.h"

unsigned int g_max_entities;
unsigned int g_last_allocated_entity;
Entity *g_entities;

void free_entity_system();

void init_entity_system(unsigned int max_entities) {
	g_max_entities = max_entities;
	g_entities = calloc(sizeof(Entity), max_entities);
	g_last_allocated_entity = max_entities-1;
	atexit(free_entity_system);
}

void free_entity(Entity *ent);

void free_entity_system() {
	for(int i = 0; i < g_max_entities; i++) free_entity(&g_entities[i]);
	free(g_entities);
}

Entity *allocate_entity() {
	for(int i = (g_last_allocated_entity+1)%g_max_entities;
			i != g_last_allocated_entity;
			i = (i+1)%g_max_entities) {
		if(!g_entities[i].inuse) {
			g_last_allocated_entity = i;
			Entity *entity = &g_entities[i];
			memset(entity, 0, sizeof(Entity));
			entity->inuse = true;
			entity->new = true;
			return entity;
		}
	}
	return NULL;
}

void clear_entities() {
	for(int i = 0; i < g_max_entities; i++) free_entity(&g_entities[i]);
	memset(g_entities, 0, sizeof(Entity) * g_max_entities);
}

void clear_ui_group(int group) {
	for(int i = 0; i < g_max_entities; i++) {
		Entity *ent = &g_entities[i];
		if(ent->group == group) {
			free_entity(ent);
		}
	}
}

void free_entity(Entity *ent) {
	if(!ent->inuse) {
		return;
	}
	if(ent->cleanup) {
		ent->cleanup(ent);
	}
	ent->inuse = false;
}

extern Uint32 g_prev_mouse_buttons;
extern Uint32 g_mouse_buttons;
extern int g_mouse_x;
extern int g_mouse_y;
extern int g_prev_mouse_x;
extern int g_prev_mouse_y;

Bool entity_rect_test(Entity *ent, int x, int y) {
	return x > ent->position.x && x < ent->position.x+ent->size.x &&
			y > ent->position.y && y < ent->position.y+ent->size.y;
}

void entity_mouse_events(Entity *ent) {
	if(!ent->mouse_enter && !ent->mouse_exit) {
		return;
	}
	Bool is_in_rect = entity_rect_test(ent, g_mouse_x, g_mouse_y);
	Bool was_in_rect = entity_rect_test(ent, g_prev_mouse_x, g_prev_mouse_y);
	if(ent->new) {
		was_in_rect = false;
		ent->new = false;
	}

	if(ent->mouse_enter && is_in_rect && !was_in_rect) {
		ent->mouse_enter(ent);
	}
	if(ent->mouse_exit && !is_in_rect && was_in_rect) {
		if(ent->clicked && ent->mouse_up) {
			ent->mouse_up(ent);
		}
		ent->mouse_exit(ent);
		ent->clicked = false;
	}
	if(is_in_rect && (g_mouse_buttons & 1)) {
		ent->clicked = true;
		if(ent->mouse_down) {
			ent->mouse_down(ent);
		}
	}
	if(is_in_rect && !(g_mouse_buttons & 1) && ent->clicked) {
		ent->clicked = false;
		if(ent->mouse_up) {
			ent->mouse_up(ent);
		}
		if(ent->click) {
			ent->click(ent);
		}
	}
}

void entity_frame() {
	for(int i = 0; i < g_max_entities; i++) {
		Entity *ent = &g_entities[i];
		if(!ent->inuse) {
			continue;
		}
		entity_mouse_events(ent);
		if(ent->think) {
			ent->think(ent);
		}
	}
	for(int i = 0; i < g_max_entities; i++) {
		Entity *ent = &g_entities[i];
		if(!ent->inuse) {
			continue;
		}
		if(ent->update) {
			ent->update(ent);
		}
		if(!ent->inuse) {
			continue;
		}
		if(ent->draw) {
			ent->draw(ent);
		}
	}
}

int get_entity_id(Entity *ent) {
	return ent-g_entities;
}

Entity *get_entity(int id) {
	return &g_entities[id];
}

void button_mouse_enter(Entity *button) {
	button->bg_color = gfc_color(0.2, 0.2, 0.2, 1.0);
}

void button_mouse_exit(Entity *button) {
	button->bg_color = gfc_color(0.1, 0.1, 0.1, 1.0);
}

void button_mouse_down(Entity *button) {
	button->bg_color = gfc_color(0.0, 0.0, 0.0, 1.0);
}

void button_mouse_up(Entity *button) {
	button->bg_color = gfc_color(0.2, 0.2, 0.2, 1.0);
}

void draw_text_rect(Entity *ent) {
	Rect rect = gfc_rect(ent->position.x, ent->position.y, ent->size.x, ent->size.y);
	gf2d_draw_rect_filled(rect, ent->bg_color);
	gf2d_draw_rect(rect, ent->border_color);
	if(ent->text[0] != 0) {
		font_render_aligned(ent->text, ent->font_size, ent->color,
				sdl_rect(ent->position.x, ent->position.y, ent->size.x, ent->size.y),
				ent->text_align_x, ent->text_align_y);
	}
}

Entity *create_label(Vector2D position, TextAlign text_align_x, TextAlign text_align_y) {
	Entity *label = allocate_entity();
	label->text_align_x = text_align_x;
	label->text_align_y = text_align_y;
	label->position = position;
	label->font_size = 2;
	label->color = gfc_color(1.0, 1.0, 1.0, 1.0);
	label->draw = draw_text_rect;
	return label;
}

Entity *create_button(Vector2D position, Vector2D size, char *text) {
	Entity *button = allocate_entity();
	button->bg_color = gfc_color(0.1, 0.1, 0.1, 1.0);
	button->border_color = gfc_color(1.0, 1.0, 1.0, 1.0);
	button->color = gfc_color(1.0, 1.0, 1.0, 1.0);
	button->draw = draw_text_rect;
	button->mouse_enter = button_mouse_enter;
	button->mouse_exit = button_mouse_exit;
	button->mouse_down = button_mouse_down;
	button->mouse_up = button_mouse_up;
	button->position = vector2d(position.x-size.x/2, position.y-size.y/2);
	button->size = size;
	button->font_size = 5;
	button->text_align_x = CENTER;
	button->text_align_y = CENTER;
	memcpy(button->text, text, strlen(text)+1);
	return button;
}